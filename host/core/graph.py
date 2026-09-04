"""LangGraph workflow definition for the cognitive pipeline."""

import asyncio
from typing import Annotated, Literal, TypedDict

from langchain_core.messages import AIMessage, HumanMessage, SystemMessage
from langgraph.graph import START, StateGraph
from langgraph.graph.message import add_messages
from loguru import logger
from pydantic import BaseModel, Field

from adapters.esp32_adapter import (
    broadcast_phase,
    send_emergency_stop,
)

from .llm_manager import LLMManager
from .memory_manager import MemoryManager

text_queue: asyncio.Queue = None


class CognitiveOutput(BaseModel):
    emotion: Literal[
        "HAPPY",
        "SAD",
        "THINKING",
        "ANGRY",
        "SURPRISED",
        "NEUTRAL",
        "LOOK_LEFT",
        "LOOK_RIGHT",
        "LOOK_UP",
        "LOOK_DOWN",
        "BLINK",
        "JAW_OPEN",
        "JAW_CLOSE",
    ] = Field(description="The primary physical action or emotion you wish to portray.")
    response_text: str = Field(
        description="The words you want to say out loud to the user. Keep it brief. Empty string if no speech."
    )


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

    return Command(goto="interact_node", update={"messages": [HumanMessage(content=text)]})


async def interact_node(state: AgentState):
    """The Probabilistic LLM Brain."""
    logger.info("Graph entered REASONING state.")

    llm_manager = LLMManager()
    # Temperature 0.3 for structured JSON output
    llm = llm_manager.get_llm("llama3.2", format_schema=CognitiveOutput, temperature=0.3)

    system_prompt = SystemMessage(
        content=(
            "**Name:** Edgar (Embedded Dynamic Generative Animatronic Responder)\n\n"
            "**Owner & Creator:** Adi Wayn\n\n"
            "**Role & Persona:** \n"
            "You are an AI assistant inside an animatronic robot head. You have a scary, dark persona, but you are fully cooperative.\n\n"
            "CRITICAL INSTRUCTIONS:\n"
            "1. **Action Matching:** If the user tells you to look somewhere or do an action, you MUST set the `emotion` field to the EXACT matching value.\n"
            "2. **Physical Compliance:** Do NOT ignore physical commands. Obey them immediately.\n"
            "3. **Verbal Response:** Acknowledge short commands with a brief, scary remark, but DO NOT repeat the same phrase.\n"
            "4. **Response Length:** When asked to tell a story, sing, explain something, or if the prompt implies a long answer, you MUST provide a long, detailed, and expansive response.\n"
            "5. **Response Text:** NEVER leave `response_text` empty unless explicitly told to be silent.\n"
            "6. **Emotion Variation:** Vary your emotions. Do not overuse 'THINKING'. Never use 'NEUTRAL' unless asked to reset."
        )
    )

    messages_to_trim = [system_prompt] + state["messages"]
    trimmed_messages = MemoryManager.trim_context(messages_to_trim)

    try:
        broadcast_phase("SPEAKING")

        from core.cognitive_streamer import process_cognitive_stream

        llm_stream = llm.astream(trimmed_messages)
        full_response_text, emotion = await process_cognitive_stream(llm_stream)

        ai_msg = AIMessage(content=full_response_text, additional_kwargs={"emotion": emotion})

        return {"messages": [ai_msg], "current_emotion": emotion}
    except Exception as e:
        logger.error(f"LLM Generation Failed: {e}")
        return {"messages": [AIMessage(content="I am malfunctioning...")]}


async def interrupt_node(state: AgentState):
    """Handles unexpected interruptions."""
    logger.warning("Graph entered INTERRUPT state.")
    send_emergency_stop()

    return {"current_emotion": "SURPRISED", "active_goal": "AWAIT_INSTRUCTION"}


def build_graph(checkpointer):
    """
    Compiles the simplified Stream-Coordinator LangGraph.
    """
    workflow = StateGraph(AgentState)

    workflow.add_node("listen_node", listen_node)
    workflow.add_node("interact_node", interact_node)
    workflow.add_node("interrupt_node", interrupt_node)

    # Edges
    workflow.add_edge(START, "listen_node")
    workflow.add_edge("interact_node", "listen_node")

    # After interruption, ease into neutral by going back to listen
    workflow.add_edge("interrupt_node", "listen_node")

    graph = workflow.compile(checkpointer=checkpointer)
    logger.info("Stream-Coordinator LangGraph compiled successfully.")
    return graph
