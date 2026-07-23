import logging
import json
from typing import TypedDict, Annotated, Sequence, Literal
from langchain_core.messages import BaseMessage, HumanMessage, AIMessage, SystemMessage
from langgraph.graph import StateGraph, START, END
from langgraph.graph.message import add_messages
from langchain.agents import create_agent

from pydantic import BaseModel, Field

from .llm_manager import LLMManager
from .memory import get_checkpointer
from tools.esp32_adapter import send_kinematic_intent

logger = logging.getLogger(__name__)

# State schema for the graph
class AgentState(TypedDict):
    messages: Annotated[list[BaseMessage], add_messages]
    final_response: str
    kinematic_intent: str

# Pydantic schema for strict JSON output from Fast LLM
class AnimatronicResponse(BaseModel):
    speech: str = Field(description="The conversational text to speak back to the user.")
    emotion: Literal["HAPPY", "SAD", "SURPRISED", "ANGRY", "NEUTRAL", "LOOK_LEFT", "LOOK_RIGHT", "LOOK_UP", "LOOK_DOWN"] = Field(
        description="The physical emotion or movement intent."
    )

def router_node(state: AgentState) -> Literal["fast_generator", "supervisor"]:
    """
    Router (Triage) Pattern: Evaluates if the request is casual talk or requires tools/reasoning.
    """
    last_message = state["messages"][-1].content.lower()
    
    # Simple heuristic for fast routing (can be replaced by a small LLM call if latency allows)
    action_keywords = ["move", "look", "act", "express", "show", "turn", "head", "weather", "search"]
    
    if any(keyword in last_message for keyword in action_keywords):
        logger.info("Router decided: Complex request -> Supervisor")
        return "supervisor"
    
    logger.info("Router decided: Casual talk -> Fast Generator")
    return "fast_generator"

def fast_generator_node(state: AgentState) -> AgentState:
    """
    Directly generates a response using Llama 3 for casual conversation (low latency).
    """
    logger.debug("Executing Fast Generator Node...")
    llm_manager = LLMManager()
    # Using Llama 3 for fast, native GPU inference
    llm = llm_manager.get_llm("llama3", format_schema=AnimatronicResponse)
    
    # Format history for context
    sys_prompt = SystemMessage(content="You are an animatronic head. Respond casually and naturally. Keep responses short and conversational.")
    messages = [sys_prompt] + state["messages"]
    
    response = llm.invoke(messages)
    
    # The structured output returns the Pydantic model directly
    try:
        speech = response.speech
        emotion = response.emotion
    except AttributeError:
        # Fallback if parsing fails
        speech = "I didn't quite catch that."
        emotion = "NEUTRAL"
        
    return {"final_response": speech, "kinematic_intent": emotion}

def supervisor_node(state: AgentState) -> AgentState:
    """
    Supervisor (Orchestrator) Pattern: Uses LangGraph's prebuilt ReAct agent to call tools.
    """
    logger.debug("Executing Supervisor Node...")
    llm_manager = LLMManager()
    llm = llm_manager.get_llm("llama3", temperature=0.2)
    
    tools = [send_kinematic_intent]
    
    # We create a sub-graph react agent using the new supported create_agent method
    agent = create_agent(llm, tools)
    
    # Run the react agent
    result = agent.invoke({"messages": state["messages"]})
    
    # Extract the final message from the react agent
    final_message = result["messages"][-1].content
    
    # Because tools are handled within the react loop, the ESP32 is already called.
    return {"final_response": final_message, "kinematic_intent": "NEUTRAL"}

def reflection_node(state: AgentState) -> AgentState:
    """
    Reflection (Self-Correction) Pattern: Verifies the emotion intent is safe.
    """
    valid_emotions = ["HAPPY", "SAD", "SURPRISED", "ANGRY", "NEUTRAL", "LOOK_LEFT", "LOOK_RIGHT", "LOOK_UP", "LOOK_DOWN"]
    intent = state.get("kinematic_intent", "NEUTRAL")
    
    if intent not in valid_emotions:
        logger.warning(f"Reflection caught invalid intent: {intent}. Forcing to NEUTRAL.")
        return {"kinematic_intent": "NEUTRAL"}
        
    return state # Unchanged if valid

def build_graph():
    """Compiles the LangGraph Agentic Ecosystem."""
    workflow = StateGraph(AgentState)
    
    workflow.add_node("fast_generator", fast_generator_node)
    workflow.add_node("supervisor", supervisor_node)
    workflow.add_node("reflection", reflection_node)
    
    workflow.add_conditional_edges(
        START,
        router_node,
        {
            "fast_generator": "fast_generator",
            "supervisor": "supervisor"
        }
    )
    
    workflow.add_edge("fast_generator", "reflection")
    workflow.add_edge("supervisor", "reflection")
    workflow.add_edge("reflection", END)
    
    checkpointer = get_checkpointer()
    graph = workflow.compile(checkpointer=checkpointer)
    logger.info("Agentic Graph compiled successfully with SQLite memory.")
    return graph

# Singleton instance
agentic_graph = build_graph()
