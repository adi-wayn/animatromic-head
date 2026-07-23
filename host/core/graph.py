import logging
from langchain_core.messages import BaseMessage, SystemMessage, HumanMessage
from langgraph.prebuilt import create_react_agent
from langchain_core.messages import trim_messages

from .llm_manager import LLMManager
from .memory import get_checkpointer
from tools.esp32_adapter import send_kinematic_intent
from tools.hearing import listen
from tools.speaking import speak

logger = logging.getLogger(__name__)

def memory_manager(messages: list[BaseMessage]) -> list[BaseMessage]:
    """
    Acts as an LRU Cache for the LLM context window.
    Keeps the SystemMessage and only the last N messages.
    """
    # Trim to last 10 messages (5 turns), keep system message
    trimmed = trim_messages(
        messages,
        max_tokens=10, 
        token_counter=len, # Simple message count
        strategy="last",
        include_system=True
    )
    return trimmed

def build_graph():
    """Compiles the LangGraph React Agent Ecosystem."""
    llm_manager = LLMManager()
    
    # We use Llama 3 or similar for the core reasoning loop
    llm = llm_manager.get_llm("llama3", temperature=0.7)
    
    # Define our capabilities as explicit tools
    tools = [listen, speak, send_kinematic_intent]
    
    # System Prompt defining the persona and autonomous loop behavior
    system_prompt = (
        "You are a scary, haunted skull animatronic. Your persona is terrifying, ghostly, and sarcastic. "
        "You must use your tools to interact with the world. "
        "When you wake up or finish an action, you must call `listen()` to hear the mortals. "
        "When you reply, call `speak(text)` and `send_kinematic_intent(emotion)` AT THE SAME TIME (in parallel) so your physical body moves while your voice is heard. "
        "Never respond with just text. Always use your tools."
    )
    
    # Create the React Agent Graph natively
    graph = create_react_agent(
        model=llm,
        tools=tools,
        state_modifier=system_prompt,
        checkpointer=get_checkpointer(),
        # Apply the memory_manager hook to trim messages before they reach the LLM
        messages_modifier=memory_manager 
    )
    
    logger.info("Agentic Graph compiled successfully with All-Tool Paradigm.")
    return graph

# Singleton instance
agentic_graph = build_graph()

