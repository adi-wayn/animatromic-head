"""Stream processor for LLM cognitive output."""

import asyncio
import queue

from loguru import logger

from adapters.esp32_adapter import send_kinematic_intent
from adapters.speaking import speak_stream
from audio.tts.dual_tts_manager import dual_tts_manager


async def process_cognitive_stream(llm_stream) -> tuple[str, str]:
    """
    Processes the raw token stream from the LLM, extracting emotions and
    streaming sentences to the TTS engine concurrently.
    """
    sentence_queue = queue.Queue()
    tts_task = asyncio.create_task(asyncio.to_thread(speak_stream, sentence_queue))

    buffer = ""
    emotion = "NEUTRAL"
    emotion_dispatched = False
    full_response_text = ""
    prev_text = ""
    sentence_delimiters = {".", "!", "?", ",", "-"}

    print("\n\033[96m[Animatronic Head]: \033[0m", end="", flush=True)

    async for chunk in llm_stream:
        # Abort if the user interrupts us while speaking
        if dual_tts_manager.is_interrupted:
            logger.info("LLM generation aborted due to user interrupt.")
            break

        # Parse dynamic emotions from the Pydantic chunk
        if hasattr(chunk, "emotion") and chunk.emotion and not emotion_dispatched:
            emotion = chunk.emotion.upper()
            send_kinematic_intent(emotion, 0.8)
            logger.info(f"Dispatched Dynamic Emotion: {emotion}")
            emotion_dispatched = True
        elif isinstance(chunk, dict) and chunk.get("emotion") and not emotion_dispatched:
            emotion = chunk["emotion"].upper()
            send_kinematic_intent(emotion, 0.8)
            logger.info(f"Dispatched Dynamic Emotion: {emotion}")
            emotion_dispatched = True

        current_text = (
            getattr(chunk, "response_text", "")
            if hasattr(chunk, "response_text")
            else (chunk.get("response_text", "") if isinstance(chunk, dict) else "")
        )
        if not current_text:
            continue

        # Extract the new text generated in this chunk
        diff = current_text[len(prev_text) :]
        if diff:
            buffer += diff
            prev_text = current_text

            # Parse sentences from the buffer
            while True:
                first_delim = min(
                    [buffer.find(d) for d in sentence_delimiters if buffer.find(d) != -1],
                    default=-1,
                )
                if first_delim != -1:
                    sentence = buffer[: first_delim + 1].strip()
                    buffer = buffer[first_delim + 1 :]
                    if len(sentence) > 1:
                        sentence_queue.put(sentence)
                        full_response_text += sentence + " "
                        print(f"\033[93m{sentence} \033[0m", end="", flush=True)
                else:
                    break

    # Flush remaining text
    if buffer.strip():
        sentence = buffer.strip()
        sentence_queue.put(sentence)
        full_response_text += sentence
        print(f"\033[93m{sentence} \033[0m", end="", flush=True)

    print("\n", flush=True)
    sentence_queue.put(None)  # EOF marker

    # Ensure fallback intent if model failed to stream the emotion field first
    if not emotion_dispatched:
        send_kinematic_intent(emotion, 0.8)

    # Wait for audio to finish playing
    await tts_task

    return full_response_text, emotion
