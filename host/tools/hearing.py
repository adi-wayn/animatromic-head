import asyncio
import logging
from langchain_core.tools import tool
from typing import Optional

logger = logging.getLogger(__name__)

# A singleton reference to the Orchestrator's text queue
# We use this to bridge the Orchestrator's asyncio queue to the LangGraph tool
_text_queue: Optional[asyncio.Queue] = None

def set_text_queue(q: asyncio.Queue):
    global _text_queue
    _text_queue = q

@tool
async def listen() -> str:
    """
    Blocks and waits to hear what the user says.
    Call this tool whenever you finish speaking to wait for the user's next response.
    Returns the transcribed text of the user's speech.
    """
    logger.info("Agent is listening...")
    if _text_queue is None:
        logger.error("Text queue is not initialized. Cannot listen.")
        return ""
        
    logger.debug("Awaiting on text_queue...")
    text = await _text_queue.get()
    _text_queue.task_done()
    logger.info(f"Agent heard: {text}")
    return text
