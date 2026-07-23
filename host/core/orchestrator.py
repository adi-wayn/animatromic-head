import asyncio
from audio.vad import VADManager
from audio.stt import WhisperTranscriber, stt_worker
from core.graph import agentic_graph
from core.llm_manager import LLMManager
from core.memory import wipe_memory
from loguru import logger
from langchain_core.messages import HumanMessage

class CognitiveOrchestrator:
    def __init__(self):
        self.segment_queue = asyncio.Queue()
        self.text_queue = asyncio.Queue() # New queue for STT -> LLM decouple
        self.loop = asyncio.get_event_loop()
        
        # Initialize dependencies
        self.transcriber = WhisperTranscriber("base.en")
        self.vad_manager = VADManager(self.segment_queue, self.loop)
        self.llm_manager = LLMManager()
        
        # Register VAD interrupt observer
        self.vad_manager.register_interrupt_callback(self.handle_interrupt)

    def handle_interrupt(self):
        logger.warning("INTERRUPT EVENT RECEIVED! Aborting current generation and TTS playback.")
        # TODO: Add logic in Task 3.3 to clear TTS queues
        pass
        
    async def llm_worker(self):
        """Consumes text from STT and invokes the Agentic Graph."""
        logger.info("LLM Worker started. Waiting for text...")
        while True:
            text = await self.text_queue.get()
            logger.info("Orchestrator: Invoking Agentic Ecosystem...")
            config = {"configurable": {"thread_id": "1"}}
            
            def run_graph():
                return agentic_graph.invoke({"messages": [HumanMessage(content=text)]}, config=config)
            
            result = await asyncio.to_thread(run_graph)
            
            speech_response = result.get("final_response", "")
            intent = result.get("kinematic_intent", "NEUTRAL")
            
            logger.info(f"AGENT RESPONSE: {speech_response} | INTENT: {intent}")
            # Task 3.3 will pass speech_response to TTS here
            
            self.text_queue.task_done()

    async def start(self):
        logger.info("Starting Cognitive Orchestrator...")
        self.vad_manager.start()
        
        # Run workers in the background
        stt_task = asyncio.create_task(stt_worker(self.segment_queue, self.transcriber, self.text_queue))
        llm_task = asyncio.create_task(self.llm_worker())
        
        try:
            # Wait indefinitely until cancelled
            await asyncio.gather(stt_task, llm_task)
        except asyncio.CancelledError:
            logger.info("Cognitive Orchestrator shutting down.")
            self.vad_manager.stop()
            raise
