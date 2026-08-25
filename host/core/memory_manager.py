import logging
from langchain_core.messages import BaseMessage, trim_messages

logger = logging.getLogger(__name__)

class MemoryManager:
    """
    Handles context window management and LRU caching for LLM state.
    """
    @staticmethod
    def trim_context(messages: list[BaseMessage]) -> list[BaseMessage]:
        """
        Acts as an LRU Cache for the LLM context window.
        Keeps the SystemMessage and only the last N messages.
        """
        trimmed = trim_messages(
            messages,
            max_tokens=30, # Keeps the last 30 messages (approx 15 conversational turns)
            token_counter=len,
            strategy="last",
            include_system=True
        )
        return trimmed
