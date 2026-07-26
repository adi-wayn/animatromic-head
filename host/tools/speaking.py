from loguru import logger
from langchain_core.tools import tool
from audio.tts.dual_tts_manager import dual_tts_manager

@tool
def speak(text: str):
    """
    Synthesizes and speaks text using the physical speaker.
    Call this whenever you want to say something verbally to the user.
    """
    logger.info(f"Using tool: speak(text='{text}')")
    print(f"\n[BOT SAYS]: {text}\n")
    
    # Fire and forget into the DualTTSManager (it streams output live via UDP)
    dual_tts_manager.speak(text)
    return f"Finished speaking: {text}"
