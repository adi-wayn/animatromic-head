import whisper
import numpy as np
import asyncio
from loguru import logger

class WhisperTranscriber:
    def __init__(self, model_size="tiny.en"):
        import torch
        self.device = "mps" if torch.backends.mps.is_available() else "cpu"
        logger.info(f"Loading Whisper model: {model_size} on device: {self.device}")
        self.model = whisper.load_model(model_size, device=self.device)
        logger.info("Whisper model loaded successfully.")
        
    def transcribe(self, raw_audio_bytes: bytes) -> str:
        # Convert 16-bit PCM bytes to float32 NumPy array normalized between -1.0 and 1.0 for Whisper
        audio_np = np.frombuffer(raw_audio_bytes, dtype=np.int16).astype(np.float32) / 32768.0
        
        # Disable condition_on_previous_text to prevent noise hallucinations
        result = self.model.transcribe(audio_np, fp16=False, condition_on_previous_text=False)
        
        segments = result.get("segments", [])
        if not segments:
            return ""
            
        # If the average no_speech_prob is high (> 0.6), it's probably noise
        avg_no_speech = sum(s.get("no_speech_prob", 0.0) for s in segments) / len(segments)
        if avg_no_speech > 0.6:
            logger.debug(f"Dropped transcription (high no_speech_prob: {avg_no_speech:.2f})")
            return ""
            
        return result["text"].strip()

async def stt_worker(segment_queue: asyncio.Queue, transcriber: WhisperTranscriber, text_queue: asyncio.Queue):
    """Continuously processes speech segments and puts transcriptions in text_queue."""
    logger.info("STT Worker started. Waiting for speech segments...")
    while True:
        audio_bytes = await segment_queue.get()
        logger.debug(f"Speech segment captured ({len(audio_bytes)} bytes). Transcribing...")
        
        # Offload heavy inference to a thread to prevent blocking the event loop
        text = await asyncio.to_thread(transcriber.transcribe, audio_bytes)
        
        if text:
            logger.info(f"USER SAID: {text}")
            # Put transcribed text into the LLM/Orchestrator queue
            text_queue.put_nowait(text)
                
        segment_queue.task_done()
