"""Hardware adapter for communicating with the ESP32 over UDP."""

import json
import socket
import socket as _socket
import threading
from typing import Any

from loguru import logger

from protocol.messages import (
    PORT_CONTROL,
    create_emergency_stop_message,
    create_intent_message,
    create_phase_update_message,
)

# Dynamically resolve the ESP32's IP via mDNS hostname
ESP32_HOSTNAME = "animatronic-head.local"


def _resolve_esp32_ip() -> str:
    """Resolve the ESP32's IP address dynamically."""
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
        logger.warning(
            f"Could not resolve {ESP32_HOSTNAME} via mDNS. Trying UDP Broadcast Discovery..."
        )

    # UDP Broadcast Fallback
    try:
        with _socket.socket(_socket.AF_INET, _socket.SOCK_DGRAM) as sock:
            sock.setsockopt(_socket.SOL_SOCKET, _socket.SO_REUSEADDR, 1)
            try:
                # SO_REUSEPORT is required on macOS for broadcast binding
                sock.setsockopt(_socket.SOL_SOCKET, _socket.SO_REUSEPORT, 1)
            except AttributeError:
                pass
            sock.bind(("", 4213))
            sock.settimeout(15.0)
            logger.info("Listening on port 4213 for ESP32 broadcast...")
            data, addr = sock.recvfrom(1024)
            if b"ESP32_HEAD_HERE" in data:
                ip = addr[0]
                logger.info(f"Discovered ESP32 at {ip} via UDP Broadcast!")
                return ip
    except Exception as e:
        logger.error(f"UDP Broadcast discovery failed: {e}")

    logger.warning("All discovery methods failed. Using fallback simulator IP (127.0.0.1).")
    return "127.0.0.1"


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
        self._listener_thread = None
        self.audio_rx_queue = None

    def start_audio_rx(self, loop, queue):
        if self._listener_thread is not None:
            return
        self.audio_rx_queue = queue

        from protocol.messages import PORT_AUDIO_UPLINK

        def _listen():
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            try:
                sock.bind(("0.0.0.0", PORT_AUDIO_UPLINK))
                logger.info(f"UDP Audio Uplink bound on 0.0.0.0:{PORT_AUDIO_UPLINK}")
            except Exception as e:
                logger.error(f"Could not bind audio uplink: {e}")
                return

            while True:
                try:
                    data, _addr = sock.recvfrom(2048)
                    # Strip 6-byte header according to protocol
                    pcm_data = data[6:]
                    loop.call_soon_threadsafe(self.audio_rx_queue.put_nowait, pcm_data)
                except Exception:
                    pass

        self._listener_thread = threading.Thread(target=_listen, daemon=True)
        self._listener_thread.start()

    def send_raw_json(self, payload: dict[str, Any]) -> bool:
        """Sends raw JSON payload to the ESP32 over UDP."""
        try:
            data = json.dumps(payload).encode("utf-8")
            self.sock.sendto(data, (self.ip, self.port))
            # logger.debug(f"Sent UDP packet to {self.ip}:{self.port} -> {payload}")
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


def send_kinematic_intent(emotion_primary: str, intensity_level: float = 1.0) -> str:
    """
    Send a physical movement or emotional intent to the Animatronic Head hardware.
    Use this tool when you want the head to move or express an emotion physically.
    Allowed emotions/actions: HAPPY, SAD, THINKING, ANGRY, SURPRISED, NEUTRAL, LOOK_LEFT, LOOK_RIGHT, LOOK_UP, LOOK_DOWN, BLINK, JAW_OPEN, JAW_CLOSE.
    """
    return _adapter.send_intent(emotion_primary, intensity_level)


def broadcast_phase(phase: str):
    """Utility to broadcast phase changes outside of tools."""
    _adapter.broadcast_phase(phase)


def send_emergency_stop():
    """Utility to send emergency stop outside of tools."""
    _adapter.emergency_stop()
