import asyncio
import sys
from loguru import logger
from core.orchestrator import CognitiveOrchestrator

async def main():
    logger.remove()
    logger.add(sys.stdout, colorize=True, level="DEBUG")
    
    logger.info("Initializing Live Test...")
    orchestrator = CognitiveOrchestrator()
    
    # Start the orchestrator in the background
    task = asyncio.create_task(orchestrator.start())
    
    # Wait for initialization
    await asyncio.sleep(2)
    
    # Inject a STT text directly into the queue to simulate user speech
    logger.info("Injecting mock STT text into queue...")
    await orchestrator.text_queue.put("Hello you scary robot. Who are you?")
    
    # Wait for the graph to process, LLM to generate response, and TTS/Behavior to fire
    await asyncio.sleep(15)
    
    logger.info("Live test complete. Shutting down.")
    task.cancel()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except asyncio.CancelledError:
        pass
