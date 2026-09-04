"""SQLite-based memory checkpointer setup for LangGraph."""

import logging
import os

from langgraph.checkpoint.sqlite.aio import AsyncSqliteSaver

logger = logging.getLogger(__name__)

DB_PATH = os.path.join(os.path.dirname(__file__), "..", "..", "data", "memory.db")


def get_checkpointer():
    """
    Returns an async context manager for AsyncSqliteSaver.
    Ensures the data directory exists.
    """
    os.makedirs(os.path.dirname(DB_PATH), exist_ok=True)
    logger.info(f"Initializing Async SQLite Checkpointer at {DB_PATH}")
    return AsyncSqliteSaver.from_conn_string(DB_PATH)


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
