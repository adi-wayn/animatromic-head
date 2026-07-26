import socket
import json
import logging
import uuid
import time
from typing import Dict, Any, Optional
from langchain_core.tools import tool
from langgraph.prebuilt import InjectedState
from typing_extensions import Annotated
from pydantic import BaseModel, Field

from host.protocol.messages import (
    PORT_CONTROL,
    create_intent_message,
    create_emergency_stop_message,
    create_phase_update_message,
)
import socket as _socket

logger = logging.getLogger(__name__)

# Dynamically resolve the ESP32's IP via mDNS hostname
ESP32_HOSTNAME = "animatronic-head.local"

def _resolve_esp32_ip() -> str:
    """Resolve the ESP32's mDNS hostname to an IP address, with env var override."""
    import os
    env_ip = os.environ.get("ESP32_IP")
    if env_ip:
        logger.info(f"Using ESP32_IP from environment: {env_ip}")
        return env_ip
        
    try:
        info = _socket.getaddrinfo(ESP32_HOSTNAME, None, _socket.AF_INET)
        ip = info[0][4][0]
        logger.info(f"Resolved {ESP32_HOSTNAME} -> {ip}")
        return ip
    except _socket.gaierror:
        logger.warning(f"Could not resolve {ESP32_HOSTNAME}. Using fallback.")
        return "192.168.1.100"  # Fallback only if mDNS is unavailable

ESP32_IP = _resolve_esp32_ip()

class ESP32UDPAdapter:
    """
    Adapter Pattern for ESP32 UDP communications.
    Provides a high-level API over the raw UDP socket.
    """
    def __init__(self, ip: str = ESP32_IP, port: int = PORT_CONTROL):
        self.ip = ip
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        # Non-blocking or short timeout to prevent hanging the event loop
        self.sock.settimeout(0.5)

    def send_raw_json(self, payload: Dict[str, Any]) -> bool:
        """Sends raw JSON payload to the ESP32 over UDP."""
        try:
            data = json.dumps(payload).encode('utf-8')
            self.sock.sendto(data, (self.ip, self.port))
            logger.debug(f"Sent UDP packet to {self.ip}:{self.port} -> {payload}")
            return True
        except Exception as e:
            logger.error(f"Failed to send UDP packet to ESP32: {e}")
            return False

    def send_intent(self, emotion: str, intensity: float = 1.0) -> str:
        """
        High-level API for sending emotional/kinematic intents.
        """
        msg = create_intent_message(emotion_primary=emotion.upper(), intensity_level=intensity)
        success = self.send_raw_json(msg.model_dump())
        if success:
            return f"Successfully sent intent {emotion} with intensity {intensity} to hardware."
        return f"Failed to send intent {emotion} to hardware. Network error."

    def emergency_stop(self) -> None:
        """Sends an immediate INTERRUPT command."""
        msg = create_emergency_stop_message()
        self.send_raw_json(msg.model_dump())
        
    def broadcast_phase(self, phase: str) -> None:
        """Broadcasts the current conversational phase."""
        msg = create_phase_update_message(conversational_phase=phase)
        self.send_raw_json(msg.model_dump())


# Instantiate the adapter singleton
_adapter = ESP32UDPAdapter()

class CognitiveIntentSchema(BaseModel):
    emotion_primary: str = Field(description="The primary emotion (e.g., SAD, HAPPY, SURPRISED, ANGRY, NEUTRAL, LOOK_LEFT, LOOK_RIGHT)")
    intensity_level: float = Field(default=1.0, description="Float 0.0 to 1.0 representing emotion intensity")

@tool("send_kinematic_intent", args_schema=CognitiveIntentSchema)
def send_kinematic_intent(emotion_primary: str, intensity_level: float = 1.0, state: Annotated[dict, InjectedState] = None) -> str:
    """
    Send a physical movement or emotional intent to the Animatronic Head hardware.
    Use this tool when you want the head to move or express an emotion physically.
    Allowed emotions: HAPPY, SAD, SURPRISED, ANGRY, NEUTRAL, LOOK_LEFT, LOOK_RIGHT, LOOK_UP, LOOK_DOWN.
    """
    logger.debug(f"Tool Context - Current Graph State Keys: {list(state.keys()) if state else 'None'}")
    return _adapter.send_intent(emotion_primary, intensity_level)

def broadcast_phase(phase: str):
    """Utility to broadcast phase changes outside of tools."""
    _adapter.broadcast_phase(phase)

def send_emergency_stop():
    """Utility to send emergency stop outside of tools."""
    _adapter.emergency_stop()
