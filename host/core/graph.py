import logging
from langchain_core.messages import BaseMessage, SystemMessage, HumanMessage
from langgraph.graph import StateGraph, START, END
from langgraph.prebuilt import ToolNode
from langgraph.graph.message import add_messages
from typing import Annotated, TypedDict
import asyncio

from .llm_manager import LLMManager
from .memory_manager import MemoryManager
from tools.esp32_adapter import send_kinematic_intent, broadcast_phase
from tools.speaking import speak

logger = logging.getLogger(__name__)

# The orchestrator will inject the text_queue here
text_queue: asyncio.Queue = None

class AgentState(TypedDict):
    messages: Annotated[list, add_messages]

async def listen_node(state: AgentState):
    """Deterministic node that blocks until the user speaks."""
    logger.info("Graph entered LISTENING state.")
    broadcast_phase("LISTENING")
    
    if text_queue is None:
        raise ValueError("text_queue was never initialized in graph.py!")
    
    text = await text_queue.get()
    text_queue.task_done()
    
    logger.debug(f"Graph received STT text: {text}")
    return {"messages": [HumanMessage(content=text)]}

async def agent_node(state: AgentState):
    """The Probabilistic LLM Brain."""
    logger.info("Graph entered REASONING state.")
    
    llm_manager = LLMManager()
    llm = llm_manager.get_llm("llama3.1", temperature=0.7)
    tools = [speak, send_kinematic_intent]
    
    llm_with_tools = llm.bind_tools(tools)
    
    # We must prepend the system message manually since we aren't using create_react_agent
    system_prompt = SystemMessage(
        content=(
            "You are a scary, haunted skull animatronic. Your persona is terrifying, ghostly, and sarcastic. "
            "You MUST use your tools to interact with the physical world. "
            "When you reply verbally, call `speak(text)` and `send_kinematic_intent(emotion)` AT THE SAME TIME (in parallel) "
            "so your physical body moves in sync while your voice is heard. "
            "HOWEVER, you can also move WITHOUT speaking (e.g. idle movements, looking around, reacting silently) by ONLY calling `send_kinematic_intent(emotion)`. "
            "Never respond with just plain text. ALWAYS use your tools to act, whether speaking, moving, or both."
        )
    )
    
    # Ensure system prompt is the first message before trimming
    messages_to_trim = [system_prompt] + state["messages"]
    trimmed_messages = MemoryManager.trim_context(messages_to_trim)
    
    response = await llm_with_tools.ainvoke(trimmed_messages)
    return {"messages": [response]}

def route_after_agent(state: AgentState):
    """Routes based on whether the LLM decided to use tools."""
    last_msg = state["messages"][-1]
    if hasattr(last_msg, "tool_calls") and last_msg.tool_calls:
        logger.info("Graph routing to ACTION state (Tool Execution).")
        
        # Dynamically determine the phase based on which tools were used
        tool_names = [call["name"] for call in last_msg.tool_calls]
        if "speak" in tool_names:
            broadcast_phase("SPEAKING")
        else:
            broadcast_phase("MOVING")
            
        return "action_node"
    
    logger.warning("Agent did not use any tools! Routing back to LISTENING.")
    return "listen_node"

def build_graph(checkpointer):
    """
    Compiles the LangGraph Hybrid State Machine.
    """
    workflow = StateGraph(AgentState)
    
    # The tools node from langgraph.prebuilt automatically executes the tools defined
    tools = [speak, send_kinematic_intent]
    action_node = ToolNode(tools)
    
    workflow.add_node("listen_node", listen_node)
    workflow.add_node("agent_node", agent_node)
    workflow.add_node("action_node", action_node)
    
    workflow.add_edge(START, "listen_node")
    workflow.add_edge("listen_node", "agent_node")
    
    workflow.add_conditional_edges(
        "agent_node", 
        route_after_agent, 
        {"action_node": "action_node", "listen_node": "listen_node"}
    )
    
    # After executing the tools, it can go back to the agent to observe tool output
    # or immediately cycle back to listening. A React agent usually goes back to the agent.
    # To keep it strict Perceive->Think->Act, we can just route back to listen_node,
    # but the LLM might need to see the tool success messages.
    # We will route back to agent_node to allow it to evaluate the tool output.
    workflow.add_edge("action_node", "agent_node")
    
    graph = workflow.compile(checkpointer=checkpointer)
    logger.info("Hybrid State Machine Graph compiled successfully.")
    return graph

