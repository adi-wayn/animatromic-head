import asyncio
import numpy as np
import torch
import collections
from loguru import logger
from host.protocol.messages import AUDIO_SAMPLE_RATE_HZ

class UDPVADBridge:
    """
    Replaces the pyaudio callback loop. Reads raw PCM chunks from a 
    UDP queue, runs Silero VAD, and emits complete speech segments 
    to the STT queue — same contract as the old VADManager.
    """
    def __init__(self, audio_queue: asyncio.Queue, segment_queue: asyncio.Queue):
        self.audio_queue = audio_queue
        self.segment_queue = segment_queue
        
        logger.info("Loading Silero VAD...")
        self.model, _ = torch.hub.load(
            repo_or_dir='snakers4/silero-vad',
            model='silero_vad',
            force_reload=False,
            trust_repo=True
        )
        self.model.eval()
        logger.info("Silero VAD loaded.")
        
        self.RATE = AUDIO_SAMPLE_RATE_HZ
        self.CHUNK_SIZE = 512  # Silero VAD natively supports 512 samples
        self.CHUNK_DURATION_MS = int((self.CHUNK_SIZE / self.RATE) * 1000)
        
        self.num_padding_frames = int(600 / self.CHUNK_DURATION_MS)
        self.ring_buffer = collections.deque(maxlen=self.num_padding_frames)
        
        self.triggered = False
        self.voiced_frames = []
        self.silence_limit_frames = int(1500 / self.CHUNK_DURATION_MS)
        self.silence_counter = 0
        
        self.interrupt_callbacks = []
        self._pcm_remainder = b""

    def register_interrupt_callback(self, callback):
        self.interrupt_callbacks.append(callback)

    async def run(self):
        """Main loop: pulls PCM from UDP queue, runs VAD."""
        logger.info(f"UDPVADBridge started. Rate: {self.RATE}Hz")
        while True:
            raw_chunk = await self.audio_queue.get()
            
            # Combine with any leftover bytes from the previous packet
            raw_chunk = self._pcm_remainder + raw_chunk
            self._pcm_remainder = b""
            
            # Silero needs exactly 512 samples (1024 bytes) at a time
            offset = 0
            frame_bytes = self.CHUNK_SIZE * 2  # 16-bit = 2 bytes per sample
            while offset + frame_bytes <= len(raw_chunk):
                frame = raw_chunk[offset:offset + frame_bytes]
                offset += frame_bytes
                self._process_frame(frame)
            
            # Save remainder for next packet
            if offset < len(raw_chunk):
                self._pcm_remainder = raw_chunk[offset:]

    def _process_frame(self, frame_data: bytes):
        """Process a single 512-sample frame through Silero VAD."""
        audio_np = np.frombuffer(frame_data, dtype=np.int16).astype(np.float32) / 32768.0
        tensor = torch.from_numpy(audio_np)
        
        with torch.no_grad():
            speech_prob = self.model(tensor, self.RATE).item()
        
        is_speech = speech_prob > 0.5
        
        if not self.triggered:
            self.ring_buffer.append((frame_data, is_speech))
            num_voiced = len([f for f, speech in self.ring_buffer if speech])
            
            if num_voiced >= 8:
                self.triggered = True
                self.voiced_frames.extend([f for f, s in self.ring_buffer])
                self.ring_buffer.clear()
                self.silence_counter = 0
                logger.debug("Speech started. Publishing INTERRUPT event.")
                for cb in self.interrupt_callbacks:
                    cb()
        else:
            self.voiced_frames.append(frame_data)
            if is_speech:
                self.silence_counter = 0
            else:
                self.silence_counter += 1
                
            if self.silence_counter >= self.silence_limit_frames:
                self.triggered = False
                self.model.reset_states()
                
                segment = b"".join(self.voiced_frames)
                self.voiced_frames = []
                self.silence_counter = 0
                
                if len(segment) >= 12800:
                    self.segment_queue.put_nowait(segment)
                    logger.debug(f"Speech segment emitted ({len(segment)} bytes)")
                else:
                    logger.debug(f"Discarded short segment ({len(segment)} bytes)")
