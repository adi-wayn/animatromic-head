import concurrent.futures
import io
import wave
from unittest.mock import MagicMock, patch

import numpy as np

from audio.tts.dual_tts_manager import DualTTSManager


@patch("audio.tts.dual_tts_manager.XTTSStrategy")
@patch("audio.tts.dual_tts_manager._socket.socket")
def test_dual_tts_manager_speak_success(mock_socket, mock_xtts_class):
    """Test DualTTSManager calls XTTS and streams audio via lip-sync correctly."""
    # Create mock XTTS
    mock_xtts_instance = MagicMock()
    mock_xtts_class.return_value = mock_xtts_instance

    # Generate some dummy WAV data to simulate XTTS output
    wav_io = io.BytesIO()
    with wave.open(wav_io, "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(16000)
        # Write 0.1s of silence
        dummy_pcm = np.zeros(1600, dtype=np.int16).tobytes()
        wf.writeframes(dummy_pcm)

    dummy_wav_bytes = wav_io.getvalue()

    # Configure mock to return the dummy WAV bytes
    mock_xtts_instance.synthesize.return_value = dummy_wav_bytes

    # Initialize Manager
    manager = DualTTSManager()

    # Spy on _play_with_lip_sync
    with (
        patch.object(
            manager, "_play_with_lip_sync", wraps=manager._play_with_lip_sync
        ) as mock_play,
        patch("audio.tts.dual_tts_manager._adapter") as mock_adapter,
    ):
        manager.speak("Hello world")

        # Assertions
        mock_xtts_instance.synthesize.assert_called_once_with("Hello world")
        mock_play.assert_called_once_with(dummy_wav_bytes)
        # Should close the jaw at the end
        mock_adapter.send_intent.assert_called_with("JAW", intensity=0.0)


@patch("audio.tts.dual_tts_manager.XTTSStrategy")
def test_dual_tts_manager_timeout(mock_xtts_class):
    """Test DualTTSManager handles XTTS timeout correctly without crashing."""
    mock_xtts_instance = MagicMock()
    mock_xtts_class.return_value = mock_xtts_instance

    # Force a timeout exception
    def slow_synth(text):
        raise concurrent.futures.TimeoutError("Timeout!")

    mock_xtts_instance.synthesize.side_effect = slow_synth

    manager = DualTTSManager()

    with patch.object(manager, "_play_with_lip_sync") as mock_play:
        manager.speak("Hello world")

        # It should catch the timeout, log warning, and NOT call play_with_lip_sync
        mock_xtts_instance.stop.assert_called_once()
        mock_play.assert_not_called()
        assert manager.is_speaking_active is False
