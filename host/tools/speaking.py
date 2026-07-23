import logging
from langchain_core.tools import tool
from audio.tts.dual_tts_manager import dual_tts_manager

logger = logging.getLogger(__name__)

@tool
def speak(text: str) -> str:
    """
    Synthesize and play speech from the given text.
    Call this tool whenever you want to reply to the user.
    """
    logger.info(f"Agent speaks: {text}")
    dual_tts_manager.speak(text)
    return "Finished speaking."
