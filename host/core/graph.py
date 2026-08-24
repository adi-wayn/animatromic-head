from loguru import logger
from langchain_core.messages import BaseMessage, SystemMessage, HumanMessage, AIMessage
from langgraph.graph import StateGraph, START, END
from langgraph.graph.message import add_messages
from typing import Annotated, TypedDict, Optional, Literal
from pydantic import BaseModel, Field
import asyncio
import json

from .llm_manager import LLMManager
from .memory_manager import MemoryManager
from adapters.esp32_adapter import send_kinematic_intent, broadcast_phase, send_emergency_stop
from adapters.speaking import speak
from audio.tts.dual_tts_manager import dual_tts_manager

text_queue: asyncio.Queue = None

class CognitiveOutput(BaseModel):
    emotion: Literal['HAPPY', 'SAD', 'THINKING', 'ANGRY', 'SURPRISED', 'NEUTRAL', 'LOOK_LEFT', 'LOOK_RIGHT', 'LOOK_UP', 'LOOK_DOWN', 'BLINK', 'JAW_OPEN', 'JAW_CLOSE'] = Field(description="The primary physical action or emotion you wish to portray.")
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
    from langgraph.types import Command
    return Command(goto="agent_node", update={"messages": [HumanMessage(content=text)]})

async def agent_node(state: AgentState):
    """The Probabilistic LLM Brain."""
    logger.info("Graph entered REASONING state.")
    
    llm_manager = LLMManager()
    # Temperature 0.3 for structured JSON output
    llm = llm_manager.get_llm("llama3.2", format_schema=CognitiveOutput, temperature=0.3)
    
    system_prompt = SystemMessage(
        content=(
            "You are a highly intelligent, cooperative AI assistant contained within an animatronic robot head. "
            "Crucially, you have a dark, terrifying, and villainous persona. You speak with a scary, commanding tone, "
            "but you are fully cooperative: you will accurately answer questions (math, science, identity, purpose) "
            "and strictly obey user commands. "
            "You can control your physical head's movements and emotions through the 'emotion' field in your response schema. "
            "If you feel certain emotion or you want to move based on the user input or the user commands you to perform a specific action (e.g., 'look left', 'be sad'), you MUST set the "
            "emotion field to match that action exactly. Otherwise, pick the most appropriate "
            "emotion for your scary response (e.g. 'ANGRY', 'THINKING', 'LOOK_UP'). "
            "NEVER choose 'NEUTRAL' unless the user explicitly asks you to reset your pose."
        )
    )
    
    messages_to_trim = [system_prompt] + state["messages"]
    trimmed_messages = MemoryManager.trim_context(messages_to_trim)
    
    try:
        broadcast_phase("SPEAKING")
        print("\n\033[96m[Animatronic Head]: \033[0m", end="", flush=True)
        
        # Wait for the FULL structured JSON response (no streaming)
        response: CognitiveOutput = await llm.ainvoke(trimmed_messages)
        
        emotion = response.emotion.upper()
        response_text = response.response_text
        
        # Dispatch kinematic intent immediately
        send_kinematic_intent(emotion, 0.8)
        
        if response_text:
            print(f"\033[95m[Emotion: {emotion}]\033[0m \033[93m{response_text}\033[0m\n", flush=True)
            from adapters.speaking import speak
            # Block the async loop while TTS is playing to prevent immediate transition to listen_node
            await asyncio.to_thread(speak, response_text)
            
        ai_msg = AIMessage(content=response_text, additional_kwargs={"emotion": emotion})
        
        return {
            "messages": [ai_msg],
            "current_emotion": emotion
        }
    except Exception as e:
        logger.error(f"LLM Generation Failed: {e}")
        return {"messages": [AIMessage(content="I am malfunctioning...")]}

async def behavior_node(state: AgentState):
    """The Deterministic Behavior Engine."""
    logger.info("Graph entered BEHAVIOR state.")
    
    # Extract emotion from the latest state
    emotion = state.get("current_emotion", "NEUTRAL")
    
    # 1. Hardware State Verification (Firewall)
    phys_state = state.get("robot_physical_state", {})
    cpu_load = phys_state.get("cpu_load", 0)
    
    # Example logic: If CPU load is too high, limit physical intensity
    intensity = 0.8
    if cpu_load > 85:
        logger.warning("Behavior Engine: CPU load high on Edge, clamping intensity!")
        intensity = 0.3
        
    # 2. Dispatch Movement Intent (TTS was already played dynamically by agent_node)
    broadcast_phase("MOVING")
    send_kinematic_intent(emotion, intensity)
        
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
    workflow.add_edge("agent_node", "behavior_node")
    workflow.add_edge("behavior_node", "listen_node")
    
    # After interruption, ease into neutral by going back to listen
    workflow.add_edge("interrupt_node", "listen_node")
    
    graph = workflow.compile(checkpointer=checkpointer)
    logger.info("Hybrid State Machine Graph (Phase 3) compiled successfully.")
    return graph
