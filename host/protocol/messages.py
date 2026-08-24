from pydantic import BaseModel, Field
import time
import uuid

# --- Port Constants (auto-generated from schema.json) ---
PORT_CONTROL = 4210
PORT_AUDIO_UPLINK = 4211
PORT_AUDIO_DOWNLINK = 4212
PORT_TELEMETRY = 4213

# --- Audio Format Constants (auto-generated from schema.json) ---
AUDIO_SAMPLE_RATE_HZ = 32000
AUDIO_BIT_DEPTH = 16
AUDIO_CHANNELS = 1
AUDIO_CHUNK_SIZE_BYTES = 1024
AUDIO_HEADER_SIZE_BYTES = 6

class BaseMessage(BaseModel):
    type: str
    event_id: str = Field(default_factory=lambda: str(uuid.uuid4()))
    timestamp_ms: int = Field(default_factory=lambda: int(time.time() * 1000))
    payload: dict

class IntentPayload(BaseModel):
    emotion_primary: str
    intensity_level: float = 1.0

class PhaseUpdatePayload(BaseModel):
    conversational_phase: str

class TelemetryPayload(BaseModel):
    angles: list[float]
    cpu_load: int

def create_intent_message(emotion_primary: str, intensity_level: float = 1.0) -> BaseMessage:
    return BaseMessage(
        type="INTENT",
        payload=IntentPayload(emotion_primary=emotion_primary, intensity_level=intensity_level).model_dump()
    )

def create_phase_update_message(conversational_phase: str) -> BaseMessage:
    return BaseMessage(
        type="PHASE_UPDATE",
        payload=PhaseUpdatePayload(conversational_phase=conversational_phase).model_dump()
    )

def create_emergency_stop_message() -> BaseMessage:
    return BaseMessage(type="EMERGENCY_STOP", payload={})

def create_tts_complete_message() -> BaseMessage:
    return BaseMessage(type="TTS_COMPLETE", payload={})

def create_telemetry_message(angles: list[float], cpu_load: int) -> BaseMessage:
    return BaseMessage(
        type="TELEMETRY",
        payload=TelemetryPayload(angles=angles, cpu_load=cpu_load).model_dump()
    )
