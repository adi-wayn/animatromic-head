import asyncio
from audio.vad import VADManager
from audio.stt import WhisperTranscriber, stt_worker
from core.graph import agentic_graph
from core.llm_manager import LLMManager
from core.memory import wipe_memory
from tools.hearing import set_text_queue
from audio.tts.dual_tts_manager import dual_tts_manager
from loguru import logger
from langchain_core.messages import HumanMessage

class CognitiveOrchestrator:
    def __init__(self):
        self.segment_queue = asyncio.Queue()
        self.text_queue = asyncio.Queue()
        self.loop = asyncio.get_event_loop()
        
        # Link the tool to the text_queue
        set_text_queue(self.text_queue)
        
        # Initialize dependencies
        self.transcriber = WhisperTranscriber("base.en")
        self.vad_manager = VADManager(self.segment_queue, self.loop)
        
        # Register VAD interrupt callback
        self.vad_manager.register_interrupt_callback(self.handle_interrupt)

    def handle_interrupt(self):
        logger.warning("INTERRUPT EVENT RECEIVED! Aborting current generation and TTS playback.")
        dual_tts_manager.interrupt()
        
    async def llm_worker(self):
        """
        Runs the agentic graph in a continuous loop.
        The graph will call the `listen()` tool which blocks until text_queue has data.
        """
        logger.info("LLM Worker started. Beginning autonomous loop...")
        
        config = {"configurable": {"thread_id": "1"}}
        
        # Start the loop
        while True:
            logger.debug("Invoking Agentic Ecosystem...")
            
            def run_graph():
                # We invoke with no explicit user input, letting the agent decide to use the `listen()` tool
                # However, LangGraph create_react_agent often needs a dummy message or runs until no more tools.
                # If it finishes, we restart it.
                return agentic_graph.invoke({"messages": []}, config=config)
            
            result = await asyncio.to_thread(run_graph)
            logger.debug("Graph execution completed turn. Restarting loop...")
            
            # Small yield to prevent CPU pinning if graph errors out immediately
            await asyncio.sleep(0.1)

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
