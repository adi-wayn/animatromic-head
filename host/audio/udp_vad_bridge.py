import asyncio
import numpy as np
import torch
import collections
import wave
from loguru import logger
from scipy.signal import butter, lfilter
from protocol.messages import AUDIO_SAMPLE_RATE_HZ

def butter_bandpass(lowcut, highcut, fs, order=3):
    nyq = 0.5 * fs
    low = lowcut / nyq
    high = highcut / nyq
    b, a = butter(order, [low, high], btype='band')
    return b, a

class UDPVADBridge:
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
        self.CHUNK_SIZE = 512
        self.CHUNK_DURATION_MS = int((self.CHUNK_SIZE / self.RATE) * 1000)
        
        # DSP Setup
        self.b, self.a = butter_bandpass(80.0, 3000.0, self.RATE, order=3)
        self.agc_gain = 10.0  # Initial 10x gain
        self.target_peak = 0.5 # Target float amplitude (-1.0 to 1.0)
        
        self.num_padding_frames = int(600 / self.CHUNK_DURATION_MS)
        self.ring_buffer = collections.deque(maxlen=self.num_padding_frames)
        
        self.triggered = False
        self.voiced_frames = []
        self.silence_limit_frames = int(700 / self.CHUNK_DURATION_MS)
        self.silence_counter = 0
        
        self.interrupt_callbacks = []
        self._pcm_remainder = b""
        
        # Open a proper WAV file to record what the VAD actually hears
        self.debug_wav = wave.open("latest_filtered_audio.wav", "wb")
        self.debug_wav.setnchannels(1)
        self.debug_wav.setsampwidth(2)
        self.debug_wav.setframerate(self.RATE)
        self._debug_vad_counter = 0

    def register_interrupt_callback(self, callback):
        self.interrupt_callbacks.append(callback)

    async def run(self):
        logger.info(f"UDPVADBridge started. Rate: {self.RATE}Hz")
        while True:
            raw_chunk = await self.audio_queue.get()
            
            raw_chunk = self._pcm_remainder + raw_chunk
            self._pcm_remainder = b""
            
            offset = 0
            frame_bytes = self.CHUNK_SIZE * 2
            while offset + frame_bytes <= len(raw_chunk):
                frame = raw_chunk[offset:offset + frame_bytes]
                offset += frame_bytes
                self._process_frame(frame)
            
            if offset < len(raw_chunk):
                self._pcm_remainder = raw_chunk[offset:]

    def _process_frame(self, frame_data: bytes):
        # Convert to numpy array float32 (-1.0 to 1.0)
        audio_np = np.frombuffer(frame_data, dtype=np.int16).astype(np.float32) / 32768.0
        
        # 1. Advanced Spectral Noise Reduction (Deletes white noise)
        try:
            import noisereduce as nr
            # We process tiny 32ms blocks (512 samples). We must shrink the FFT window to fit.
            audio_np = nr.reduce_noise(y=audio_np, sr=self.RATE, stationary=True, prop_decrease=1.0, n_fft=256, hop_length=64)
        except ImportError:
            pass
            
        # 2. Bandpass Filter (removes all DC drift and high-freq static)
        audio_filtered = lfilter(self.b, self.a, audio_np)
        
        # 3. AGC (Automatic Gain Control)
        current_peak = np.max(np.abs(audio_filtered))
        if current_peak > 0.01:
            # Slowly adjust gain towards target
            desired_gain = self.target_peak / current_peak
            self.agc_gain = (0.90 * self.agc_gain) + (0.10 * desired_gain)
            
        # Clamp gain to reasonable limits (1x to 30x)
        self.agc_gain = max(1.0, min(self.agc_gain, 30.0))
        
        audio_agc = audio_filtered * self.agc_gain
        audio_agc = np.clip(audio_agc, -0.99, 0.99)
        
        # 4. Save to WAV for user to hear
        final_int16 = (audio_agc * 32767).astype(np.int16).tobytes()
        self.debug_wav.writeframes(final_int16)
        
        # 5. Run Silero VAD on the processed audio
        tensor = torch.from_numpy(audio_agc.astype(np.float32))
        with torch.no_grad():
            speech_prob = self.model(tensor, self.RATE).item()
        
        is_speech = speech_prob > 0.5
        
        self._debug_vad_counter += 1
        if self._debug_vad_counter >= 30:
            logger.debug(f"[VAD] Gain: {self.agc_gain:.1f}x | Peak: {current_peak:.3f} | Speech Prob: {speech_prob:.3f}")
            self._debug_vad_counter = 0
        
        # 6. VAD Logic
        if not self.triggered:
            self.ring_buffer.append((final_int16, is_speech))
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
            self.voiced_frames.append(final_int16)
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
                    from core.metrics import turn_metrics
                    turn_metrics.mark_vad_end()
                    self.segment_queue.put_nowait(segment)
                    logger.debug(f"Speech segment emitted ({len(segment)} bytes)")
                else:
                    logger.debug(f"Discarded short segment ({len(segment)} bytes)")
