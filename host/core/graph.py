from loguru import logger
from langchain_core.messages import BaseMessage, SystemMessage, HumanMessage, AIMessage
from langgraph.graph import StateGraph, START, END
from langgraph.graph.message import add_messages
from typing import Annotated, TypedDict, Optional
from pydantic import BaseModel, Field
import asyncio
import json

from .llm_manager import LLMManager
from .memory_manager import MemoryManager
from tools.esp32_adapter import send_kinematic_intent, broadcast_phase, send_emergency_stop
from tools.speaking import speak

text_queue: asyncio.Queue = None

class CognitiveOutput(BaseModel):
    emotion: str = Field(description="The primary emotion of the response, e.g., 'SAD', 'HAPPY', 'ANGRY', 'SURPRISED', 'NEUTRAL'")
    response_text: str = Field(description="The words you want to say out loud to the user. Keep it brief. Empty string if no speech.")

class AgentState(TypedDict):
    messages: Annotated[list, add_messages]
    robot_physical_state: dict
    current_emotion: str
    active_goal: str

async def listen_node(state: AgentState):
    """Deterministic node that blocks until the user speaks."""
    logger.info("Graph entered LISTENING state.")
    broadcast_phase("LISTENING")
    
    if text_queue is None:
        raise ValueError("text_queue was never initialized in graph.py!")
    
    # Block until we get STT from the queue
    text = await text_queue.get()
    text_queue.task_done()
    
    if text == "__INTERRUPT__":
        logger.warning("listen_node intercepted an __INTERRUPT__ token. Routing to interrupt_node.")
        from langgraph.types import Command
        return Command(goto="interrupt_node")
    
    logger.debug(f"Graph received STT text: {text}")
    return {"messages": [HumanMessage(content=text)]}

async def agent_node(state: AgentState):
    """The Probabilistic LLM Brain."""
    logger.info("Graph entered REASONING state.")
    
    llm_manager = LLMManager()
    # Temperature 0 for strict JSON adherence
    llm = llm_manager.get_llm("llama3.1", temperature=0.0)
    structured_llm = llm.with_structured_output(CognitiveOutput, method="json_schema")
    
    system_prompt = SystemMessage(
        content=(
            "You are a scary, haunted skull animatronic. Your persona is terrifying, ghostly, and sarcastic. "
            "Respond to the user with a specific emotion and brief text."
        )
    )
    
    messages_to_trim = [system_prompt] + state["messages"]
    trimmed_messages = MemoryManager.trim_context(messages_to_trim)
    
    try:
        result: CognitiveOutput = await structured_llm.ainvoke(trimmed_messages)
        
        # We store the LLM's response as an AIMessage so history is maintained
        ai_msg = AIMessage(content=result.response_text, additional_kwargs={"emotion": result.emotion})
        
        return {
            "messages": [ai_msg],
            "current_emotion": result.emotion
        }
    except Exception as e:
        logger.error(f"Structured Output Parsing Failed: {e}")
        return {"messages": [AIMessage(content="I am malfunctioning...")]}

async def behavior_node(state: AgentState):
    """The Deterministic Behavior Engine."""
    logger.info("Graph entered BEHAVIOR state.")
    
    # Extract emotion and text from the latest state
    emotion = state.get("current_emotion", "NEUTRAL")
    last_msg = state["messages"][-1]
    text_to_speak = last_msg.content if isinstance(last_msg, AIMessage) else ""
    
    # 1. Hardware State Verification (Firewall)
    phys_state = state.get("robot_physical_state", {})
    cpu_load = phys_state.get("cpu_load", 0)
    
    # Example logic: If CPU load is too high, limit physical intensity
    intensity = 0.8
    if cpu_load > 85:
        logger.warning("Behavior Engine: CPU load high on Edge, clamping intensity!")
        intensity = 0.3
        
    # 2. Dispatch Movement Intent
    broadcast_phase("MOVING")
    send_kinematic_intent(emotion, intensity)
    
    # 3. Dispatch Speech
    if text_to_speak:
        broadcast_phase("SPEAKING")
        await speak(text_to_speak)
        
    return {"current_emotion": emotion}

async def interrupt_node(state: AgentState):
    """Handles unexpected interruptions."""
    logger.warning("Graph entered INTERRUPT state.")
    send_emergency_stop()
    
    return {
        "current_emotion": "SURPRISED",
        "active_goal": "AWAIT_INSTRUCTION"
    }

def build_graph(checkpointer):
    """
    Compiles the LangGraph Hybrid State Machine.
    """
    workflow = StateGraph(AgentState)
    
    workflow.add_node("listen_node", listen_node)
    workflow.add_node("agent_node", agent_node)
    workflow.add_node("behavior_node", behavior_node)
    workflow.add_node("interrupt_node", interrupt_node)
    
    # Edges
    workflow.add_edge(START, "listen_node")
    workflow.add_edge("listen_node", "agent_node")
    workflow.add_edge("agent_node", "behavior_node")
    workflow.add_edge("behavior_node", "listen_node")
    
    # After interruption, ease into neutral by going back to listen
    workflow.add_edge("interrupt_node", "listen_node")
    
    graph = workflow.compile(checkpointer=checkpointer)
    logger.info("Hybrid State Machine Graph (Phase 3) compiled successfully.")
    return graph
