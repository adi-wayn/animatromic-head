from pydantic import BaseModel, Field
import time
import uuid

class BaseMessage(BaseModel):
    """
    The strict standard envelope for all UDP packets sent to the Edge (ESP32).
    Both Host and Edge MUST adhere to this structure.
    """
    type: str
    event_id: str = Field(default_factory=lambda: str(uuid.uuid4()))
    timestamp_ms: int = Field(default_factory=lambda: int(time.time() * 1000))
    payload: dict

class IntentPayload(BaseModel):
    emotion_primary: str
    intensity_level: float = 1.0

class PhasePayload(BaseModel):
    conversational_phase: str

def create_intent_message(emotion: str, intensity: float = 1.0) -> BaseMessage:
    return BaseMessage(
        type="INTENT", 
        payload=IntentPayload(emotion_primary=emotion, intensity_level=intensity).model_dump()
    )

def create_phase_message(phase: str) -> BaseMessage:
    return BaseMessage(
        type="PHASE_UPDATE", 
        payload=PhasePayload(conversational_phase=phase).model_dump()
    )

def create_emergency_stop_message() -> BaseMessage:
    return BaseMessage(type="EMERGENCY_STOP", payload={})
