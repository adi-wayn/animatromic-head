from loguru import logger
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
    # Do not use bind_tools for this model as it's unreliable. Ask for strict JSON.
    llm = llm_manager.get_llm("llama3.1", temperature=0.7)
    
    # We must prepend the system message manually since we aren't using create_react_agent
    system_prompt = SystemMessage(
        content=(
            "You are a scary, haunted skull animatronic. Your persona is terrifying, ghostly, and sarcastic. "
            "You MUST interact with the physical world by returning a strictly formatted JSON object. "
            "Respond ONLY with valid JSON. Do not include any other text, markdown formatting, or explanation. "
            "Example format:\n"
            '{"speak": "the words you want to say out loud", "intent": "JAW", "intensity": 0.8}\n'
            "If you only want to move without speaking, set \"speak\" to an empty string. "
            "If you only want to speak without a specific intent, omit the intent fields or set to null."
        )
    )
    
    # Ensure system prompt is the first message before trimming
    messages_to_trim = [system_prompt] + state["messages"]
    trimmed_messages = MemoryManager.trim_context(messages_to_trim)
    
    response = await llm.ainvoke(trimmed_messages)
    
    # Parse the JSON response manually to extract tool calls
    import json
    import uuid
    tool_calls = []
    
    try:
        clean_content = response.content.strip()
        if clean_content.startswith("```json"):
            clean_content = clean_content[7:]
        if clean_content.startswith("```"):
            clean_content = clean_content[3:]
        if clean_content.endswith("```"):
            clean_content = clean_content[:-3]
        clean_content = clean_content.strip()
        
        import re
        clean_content = re.sub(r'\bNone\b', 'null', clean_content)
        parsed = json.loads(clean_content)
        
        # Handle Llama 3.1's natural OpenAI-style tool call format
        if isinstance(parsed, dict) and "name" in parsed and "parameters" in parsed:
            tool_calls.append({
                "name": parsed["name"],
                "args": parsed["parameters"],
                "id": str(uuid.uuid4())
            })
        elif isinstance(parsed, list):
            for item in parsed:
                if isinstance(item, dict) and "name" in item and "parameters" in item:
                    tool_calls.append({
                        "name": item["name"],
                        "args": item["parameters"],
                        "id": str(uuid.uuid4())
                    })
        else:
            # Handle our custom format
            if parsed.get("speak"):
                tool_calls.append({
                    "name": "speak",
                    "args": {"text": parsed["speak"]},
                    "id": str(uuid.uuid4())
                })
                
            if parsed.get("intent") and parsed.get("intensity") is not None:
                tool_calls.append({
                    "name": "send_kinematic_intent",
                    "args": {"emotion_primary": parsed["intent"], "intensity_level": float(parsed["intensity"])},
                    "id": str(uuid.uuid4())
                })
            
        response.content = clean_content  # keeping raw json for visibility
        if tool_calls:
            response.additional_kwargs["tool_calls"] = [] # Just satisfying langgraph structure
            response.tool_calls = tool_calls
    except Exception as e:
        logger.error(f"Failed to parse LLM JSON: {e}. Raw content: {response.content}")
        
    return {"messages": [response]}

def route_after_agent(state: AgentState):
    """Routes based on whether the LLM decided to use tools."""
    last_msg = state["messages"][-1]
    
    if last_msg.content:
        print(f"\n[LLM RAW OUTPUT]: {last_msg.content}\n")
        
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

