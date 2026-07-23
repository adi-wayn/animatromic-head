from pydantic import BaseModel, Field
import time
import uuid

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
