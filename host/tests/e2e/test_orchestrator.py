import pytest
import asyncio
from unittest.mock import patch, MagicMock
from core.orchestrator import CognitiveOrchestrator

@pytest.mark.asyncio
async def test_orchestrator_e2e_flow():
    """
    Test the E2E flow without running infinite loops.
    We'll mock STT, LLM, and TTS to ensure the queues and flags interact properly.
    """
    orchestrator = CognitiveOrchestrator()
    
    with patch("core.orchestrator.VADManager") as mock_vad, \
         patch("core.orchestrator.dual_tts_manager") as mock_tts:
         
        mock_tts.is_speaking_active = False
        
        # Test handle_interrupt logic
        # 1. When listening
        orchestrator.handle_interrupt()
        mock_tts.interrupt.assert_not_called()
        
        # 2. When speaking
        mock_tts.is_speaking_active = True
        with patch("core.orchestrator.send_emergency_stop") as mock_estop:
            orchestrator.handle_interrupt()
            mock_tts.interrupt.assert_called_once()
            mock_estop.assert_called_once()
