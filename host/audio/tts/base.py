from abc import ABC, abstractmethod
import logging

logger = logging.getLogger(__name__)

class TTSStrategy(ABC):
    """
    Strategy Pattern: Abstract base class for Text-to-Speech generation.
    """
    @abstractmethod
    def synthesize(self, text: str) -> bytes:
        """
        Synthesizes speech from text.
        Returns raw PCM audio bytes.
        """
        pass
        
    @abstractmethod
    def stop(self):
        """
        Stops any ongoing synthesis (used during interruption).
        """
        pass
