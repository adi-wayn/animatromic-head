import socket
import json
import logging
from typing import Dict, Any, Optional
from langchain_core.tools import tool

logger = logging.getLogger(__name__)

# Configurable endpoints (these should ideally be pulled from a .env or config file)
ESP32_IP = "192.168.1.100" # Placeholder, will be replaced by actual static IP
ESP32_UDP_PORT = 4210

class ESP32UDPAdapter:
    """
    Adapter Pattern for ESP32 UDP communications.
    Provides a high-level API over the raw UDP socket.
    """
    def __init__(self, ip: str = ESP32_IP, port: int = ESP32_UDP_PORT):
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
        payload = {
            "type": "INTENT",
            "emotion": emotion.upper(),
            "intensity": intensity
        }
        success = self.send_raw_json(payload)
        if success:
            return f"Successfully sent intent {emotion} with intensity {intensity} to hardware."
        return f"Failed to send intent {emotion} to hardware. Network error."

    def emergency_stop(self) -> None:
        """Sends an immediate INTERRUPT command."""
        self.send_raw_json({"type": "EMERGENCY_STOP"})


# Instantiate the adapter singleton
_adapter = ESP32UDPAdapter()

@tool
def send_kinematic_intent(emotion: str, intensity: float = 1.0) -> str:
    """
    Send a physical movement or emotional intent to the Animatronic Head hardware.
    Use this tool when the user asks the head to move, express an emotion, or perform a physical action.
    Allowed emotions: HAPPY, SAD, SURPRISED, ANGRY, NEUTRAL, LOOK_LEFT, LOOK_RIGHT, LOOK_UP, LOOK_DOWN.
    """
    return _adapter.send_intent(emotion, intensity)
