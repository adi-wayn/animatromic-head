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
from adapters.esp32_adapter import send_kinematic_intent, broadcast_phase, send_emergency_stop
from adapters.speaking import speak
from audio.tts.dual_tts_manager import dual_tts_manager

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
    from langgraph.types import Command
    return Command(goto="agent_node", update={"messages": [HumanMessage(content=text)]})

async def agent_node(state: AgentState):
    """The Probabilistic LLM Brain."""
    logger.info("Graph entered REASONING state.")
    
    llm_manager = LLMManager()
    # Temperature 0.7 for more creative/scary responses
    llm = llm_manager.get_llm("llama3.2", temperature=0.7)
    
    system_prompt = SystemMessage(
        content=(
            "You are a scary, haunted skull animatronic. Your persona is terrifying, ghostly, and sarcastic. "
            "Respond to the user with a specific emotion and brief text. "
            "You MUST start your response with the emotion in brackets, like [ANGRY] or [SAD] or [HAPPY] or [SURPRISED] or [NEUTRAL]. "
            "Then, provide the text you want to say."
        )
    )
    
    messages_to_trim = [system_prompt] + state["messages"]
    trimmed_messages = MemoryManager.trim_context(messages_to_trim)
    
    try:
        import queue
        from adapters.speaking import speak_stream
        
        sentence_queue = queue.Queue()
        # Start TTS stream consumer in a background thread to prevent event loop blocking
        tts_task = asyncio.create_task(asyncio.to_thread(speak_stream, sentence_queue))
        
        broadcast_phase("SPEAKING")
        
        full_text = ""
        emotion = "NEUTRAL"
        current_sentence = ""
        found_emotion = False
        
        async for chunk in llm.astream(trimmed_messages):
            if dual_tts_manager.is_interrupted:
                logger.warning("LLM generation aborted due to interrupt!")
                break
                
            content = chunk.content
            full_text += content
            
            if not found_emotion:
                if "]" in full_text:
                    parts = full_text.split("]", 1)
                    emotion_raw = parts[0].replace("[", "").strip()
                    emotion = emotion_raw.upper() if emotion_raw else "NEUTRAL"
                    
                    # Dispatch kinematic intent immediately as soon as emotion is known
                    send_kinematic_intent(emotion, 0.8)
                    
                    found_emotion = True
                    content = parts[1] if len(parts) > 1 else ""
                else:
                    continue # Still waiting for ]
                    
            if content:
                current_sentence += content
                # If we hit a punctuation or have enough words, push to TTS
                words = current_sentence.split()
                if any(punct in current_sentence for punct in [".", "?", "!", ",", ":", ";"]) or len(words) >= 5:
                    import re
                    # Split on punctuation if present
                    if any(punct in current_sentence for punct in [".", "?", "!", ",", ":", ";"]):
                        pieces = re.split(r'(?<=[.?!,:;]) +', current_sentence)
                        for piece in pieces[:-1]:
                            if piece.strip():
                                sentence_queue.put(piece.strip())
                        current_sentence = pieces[-1]
                    else:
                        # Split by words if it's getting too long without punctuation
                        if len(words) >= 5:
                            # Keep the last word in case it's currently being generated
                            chunk_to_send = " ".join(words[:-1])
                            if chunk_to_send.strip():
                                sentence_queue.put(chunk_to_send.strip())
                            current_sentence = words[-1] + (current_sentence[len(" ".join(words)):] if current_sentence.endswith(" ") else "")
                    
        if current_sentence.strip():
            sentence_queue.put(current_sentence.strip())
            
        sentence_queue.put(None) # Signal EOF
        
        # Wait for TTS to finish playing
        await tts_task
        
        # Clean up the output text to remove the emotion tag for memory
        clean_text = full_text.split("]", 1)[1].strip() if "]" in full_text else full_text
        ai_msg = AIMessage(content=clean_text, additional_kwargs={"emotion": emotion})
        
        return {
            "messages": [ai_msg],
            "current_emotion": emotion
        }
    except Exception as e:
        logger.error(f"Streaming LLM Failed: {e}")
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
