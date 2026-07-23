import os
import sqlite3
import logging
from langgraph.checkpoint.sqlite import SqliteSaver

logger = logging.getLogger(__name__)

DB_PATH = os.path.join(os.path.dirname(__file__), "..", "..", "data", "memory.db")

def get_checkpointer() -> SqliteSaver:
    """
    Returns a configured SqliteSaver for LangGraph state persistence.
    Ensures the data directory exists.
    """
    os.makedirs(os.path.dirname(DB_PATH), exist_ok=True)
    conn = sqlite3.connect(DB_PATH, check_same_thread=False)
    logger.info(f"Initialized SQLite Checkpointer at {DB_PATH}")
    return SqliteSaver(conn)

def wipe_memory():
    """Wipes the local SQLite memory database."""
    try:
        if os.path.exists(DB_PATH):
            os.remove(DB_PATH)
            logger.info("Memory database wiped successfully.")
        else:
            logger.info("Memory database does not exist. Nothing to wipe.")
    except Exception as e:
        logger.error(f"Failed to wipe memory: {e}")
