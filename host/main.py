"""Main entry point for the Animatronic Head host application."""

import argparse
import asyncio
import sys

from loguru import logger

from core.memory import wipe_memory
from core.orchestrator import CognitiveOrchestrator


async def main():
    parser = argparse.ArgumentParser(description="Animatronic Head Host")
    parser.add_argument(
        "--wipe",
        action="store_true",
        help="Wipe the AI's conversation memory before starting.",
    )
    args = parser.parse_args()

    if args.wipe:
        wipe_memory()

    logger.info("Initializing Animatronic Head Host Environment...")
    orchestrator = CognitiveOrchestrator()
    await orchestrator.start()


if __name__ == "__main__":
    # Configure Loguru: Output to stdout with color, and to a rotating log file
    logger.remove()  # Remove default handler
    logger.add(
        sys.stdout,
        colorize=True,
        format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level: <8}</level> | <cyan>{name}</cyan>:<cyan>{function}</cyan>:<cyan>{line}</cyan> - <level>{message}</level>",
    )
    logger.add("logs/host_{time}.log", rotation="10 MB", retention="10 days", level="DEBUG")

    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.warning("Host Environment gracefully shutting down (KeyboardInterrupt).")
