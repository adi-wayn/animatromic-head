import asyncio
from audio.vad import VADManager
from audio.stt import WhisperTranscriber, stt_worker
from core import graph
from core.llm_manager import LLMManager
from core.memory import wipe_memory, get_checkpointer
from audio.tts.dual_tts_manager import dual_tts_manager
from tools.esp32_adapter import send_emergency_stop
from loguru import logger
from langchain_core.messages import HumanMessage

class CognitiveOrchestrator:
    def __init__(self):
        self.segment_queue = asyncio.Queue()
        self.text_queue = asyncio.Queue()
        self.loop = asyncio.get_event_loop()
        
        # Link the queue to the deterministic graph node
        graph.text_queue = self.text_queue
        
        # Initialize dependencies
        self.transcriber = WhisperTranscriber("base.en")
        self.vad_manager = VADManager(self.segment_queue, self.loop)
        
        # Register VAD interrupt callback
        self.vad_manager.register_interrupt_callback(self.handle_interrupt)

    def handle_interrupt(self):
        """Called when VAD detects speech."""
        
        # Only interrupt if the bot is actually speaking
        if dual_tts_manager.is_speaking_active:
            logger.warning("INTERRUPT EVENT RECEIVED! Aborting current generation and TTS playback.")
            dual_tts_manager.interrupt()
            # SDD Requirement: Send EMERGENCY_STOP to ESP32 over UDP
            send_emergency_stop()
        else:
            pass
        
    async def llm_worker(self):
        """
        Runs the agentic graph in a continuous loop.
        The graph will start at `listen_node`, which blocks until `text_queue` has data.
        """
        logger.info("LLM Worker started. Beginning autonomous loop...")
        
        config = {"configurable": {"thread_id": "1"}}
        
        # The LangGraph `ainvoke` can be called once, or streamed. 
        # Since our graph has a cyclic loop back to `listen_node` (or we run it repeatedly),
        # we can just invoke it repeatedly. It expects new input or just runs.
        # Actually, since it's a StateGraph, we can just `ainvoke` with no initial input,
        # but LangGraph requires an initial state if we aren't resuming.
        # Let's just start the loop. The `listen_node` will block asynchronously.
        
        async with get_checkpointer() as checkpointer:
            agentic_graph = graph.build_graph(checkpointer)
            while True:
                logger.debug("Invoking Hybrid State Machine...")
                
                try:
                    # We start the graph. It goes START -> listen_node -> blocks on text_queue
                    await agentic_graph.ainvoke({"messages": []}, config=config)
                    logger.debug("Graph execution completed turn. Restarting loop...")
                except Exception as e:
                    logger.error(f"Graph execution failed: {e}")
                
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
