import pyaudio
import asyncio
import collections
import numpy as np
import torch
from loguru import logger

class VADManager:
    def __init__(self, segment_queue: asyncio.Queue, loop: asyncio.AbstractEventLoop):
        self.segment_queue = segment_queue
        self.loop = loop
        
        logger.info("Loading Silero VAD...")
        self.model, _ = torch.hub.load(repo_or_dir='snakers4/silero-vad', model='silero_vad', force_reload=False, trust_repo=True)
        self.model.eval() # Set to evaluation mode
        logger.info("Silero VAD loaded.")
        
        self.FORMAT = pyaudio.paInt16
        self.CHANNELS = 1
        self.RATE = 16000
        self.CHUNK_SIZE = 512 # Silero VAD natively supports 512 samples
        self.CHUNK_DURATION_MS = int((self.CHUNK_SIZE / self.RATE) * 1000) # 32ms
        
        self.pyaudio_instance = pyaudio.PyAudio()
        self.stream = None
        
        # We need a ring buffer to keep some pre-speech audio (e.g. 600ms to catch soft starts)
        self.num_padding_frames = int(600 / self.CHUNK_DURATION_MS)
        self.ring_buffer = collections.deque(maxlen=self.num_padding_frames)
        
        self.triggered = False
        self.voiced_frames = []
        
        # Determine silence timeout (e.g. 1500ms of silence to stop recording)
        self.silence_limit_frames = int(1500 / self.CHUNK_DURATION_MS)
        self.silence_counter = 0

    def start(self):
        logger.info(f"Starting Audio Ingestion. Rate: {self.RATE}Hz, Chunk: {self.CHUNK_DURATION_MS}ms")
        self.stream = self.pyaudio_instance.open(
            format=self.FORMAT,
            channels=self.CHANNELS,
            rate=self.RATE,
            input=True,
            frames_per_buffer=self.CHUNK_SIZE,
            stream_callback=self._audio_callback
        )
        self.stream.start_stream()

    def stop(self):
        if self.stream:
            self.stream.stop_stream()
            self.stream.close()
        self.pyaudio_instance.terminate()

    def _audio_callback(self, in_data, frame_count, time_info, status):
        try:
            # Convert 16-bit PCM bytes to float32 NumPy array normalized between -1.0 and 1.0
            audio_np = np.frombuffer(in_data, dtype=np.int16).astype(np.float32) / 32768.0
            tensor = torch.from_numpy(audio_np)
            
            # Silero VAD outputs a probability of speech
            with torch.no_grad():
                speech_prob = self.model(tensor, self.RATE).item()
            
            # Strong confidence threshold to ignore tapping/typing
            is_speech = speech_prob > 0.5
            
        except Exception as e:
            logger.error(f"VAD Error: {e}")
            return (None, pyaudio.paContinue)
            
        if not self.triggered:
            self.ring_buffer.append((in_data, is_speech))
            num_voiced = len([f for f, speech in self.ring_buffer if speech])
            
            # If a sufficient number of frames are voiced (e.g. 8 frames = 256ms), trigger recording.
            if num_voiced >= 8:
                self.triggered = True
                self.voiced_frames.extend([f for f, s in self.ring_buffer])
                self.ring_buffer.clear()
                self.silence_counter = 0
                logger.debug("Speech started.")
        else:
            self.voiced_frames.append(in_data)
            if is_speech:
                self.silence_counter = 0
            else:
                self.silence_counter += 1
                
            if self.silence_counter >= self.silence_limit_frames:
                self.triggered = False
                logger.debug("Speech ended. Emitting segment.")
                self.model.reset_states() # Reset Silero's internal RNN state
                
                # Combine all frames into one bytes object
                segment = b"".join(self.voiced_frames)
                self.voiced_frames = []
                self.silence_counter = 0
                
                # Require at least 0.4s (12800 bytes) of audio to catch short words like "What?" or "Yes"
                if len(segment) >= 12800:
                    self.loop.call_soon_threadsafe(self.segment_queue.put_nowait, segment)
                else:
                    logger.debug(f"Discarded short audio segment ({len(segment)} bytes).")

        return (None, pyaudio.paContinue)
