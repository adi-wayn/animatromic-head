"""Metrics tracking for the cognitive pipeline."""

import time

from loguru import logger


class TurnMetrics:
    """Tracks latency metrics for a single conversational turn."""

    def __init__(self):
        self.t_vad_end = 0
        self.t_stt_end = 0
        self.t_llm_end = 0
        self.t_tts_first_byte = 0

    def mark_vad_end(self):
        self.t_vad_end = time.time()

    def mark_stt_end(self):
        self.t_stt_end = time.time()
        if self.t_vad_end > 0:
            logger.info(
                f"⏱️ [METRICS] STT Transcription took: {self.t_stt_end - self.t_vad_end:.3f}s"
            )

    def mark_llm_end(self):
        self.t_llm_end = time.time()
        if self.t_stt_end > 0:
            logger.info(f"⏱️ [METRICS] LLM Generation took: {self.t_llm_end - self.t_stt_end:.3f}s")

    def mark_tts_start(self):
        self.t_tts_first_byte = time.time()
        if self.t_llm_end > 0:
            logger.info(
                f"⏱️ [METRICS] TTS Generation took: {self.t_tts_first_byte - self.t_llm_end:.3f}s"
            )

        if self.t_vad_end > 0:
            total = self.t_tts_first_byte - self.t_vad_end
            logger.info(
                f"🚀 [METRICS] === TOTAL LATENCY (User stopped speaking -> First Sound played) === : {total:.3f}s"
            )


# Global singleton
turn_metrics = TurnMetrics()
