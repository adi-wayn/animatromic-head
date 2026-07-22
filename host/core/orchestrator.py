import asyncio
from audio.vad import VADManager
from audio.stt import WhisperTranscriber, stt_worker
from loguru import logger

class CognitiveOrchestrator:
    def __init__(self):
        self.segment_queue = asyncio.Queue()
        self.loop = asyncio.get_event_loop()
        # Initialize dependencies
        self.transcriber = WhisperTranscriber("base.en")
        self.vad_manager = VADManager(self.segment_queue, self.loop)

    async def start(self):
        logger.info("Starting Cognitive Orchestrator...")
        self.vad_manager.start()
        
        # Run the STT worker in the background
        stt_task = asyncio.create_task(stt_worker(self.segment_queue, self.transcriber))
        
        try:
            # Wait indefinitely until cancelled
            await stt_task
        except asyncio.CancelledError:
            logger.info("Cognitive Orchestrator shutting down.")
            self.vad_manager.stop()
            raise
