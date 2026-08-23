import pytest
import numpy as np
from unittest.mock import MagicMock, patch
from audio.vad import VADManager

def test_vad_speech_detection():
    # Setup VAD with a mock orchestrator callback
    mock_queue = MagicMock()
    mock_loop = MagicMock()
    
    # We patch torch.hub.load so it doesn't actually download Silero
    with patch("torch.hub.load") as mock_load:
        mock_model = MagicMock()
        mock_load.return_value = (mock_model, None)
        vad = VADManager(segment_queue=mock_queue, loop=mock_loop)
    
    # Mock dual_tts_manager to not be speaking
    with patch("audio.tts.dual_tts_manager.dual_tts_manager") as mock_tts:
        mock_tts.is_speaking_active = False
        mock_tts.last_speaking_end_time = 0.0
        
        # We need a tensor output from the silero model mock
        vad.model.return_value = MagicMock(item=lambda: 0.9) # Speech confidence 0.9
        
        # Generate some dummy audio (512 samples)
        dummy_pcm = np.zeros(512, dtype=np.int16).tobytes()
        
        # Trigger it enough times to pass the 8 frame threshold
        for _ in range(8):
            vad._audio_callback(dummy_pcm, 512, None, None)
        
        # Verify speech was detected and recording started
        assert vad.triggered is True
        assert vad.silence_counter == 0

        # Now simulate silence
        vad.model.return_value = MagicMock(item=lambda: 0.1) # Confidence 0.1
        
        for _ in range(vad.silence_limit_frames + 2):
            vad._audio_callback(dummy_pcm, 512, None, None)
            
        # Verify speech segment was emitted to queue
        mock_loop.call_soon_threadsafe.assert_called_with(mock_queue.put_nowait, b"".join([dummy_pcm] * 54))
        assert vad.triggered is False
