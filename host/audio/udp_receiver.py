import asyncio
from loguru import logger
from host.protocol.messages import PORT_AUDIO_UPLINK, AUDIO_CHUNK_SIZE_BYTES

class UDPAudioReceiver(asyncio.DatagramProtocol):
    """
    Receives raw PCM audio chunks from the ESP32 INMP441 microphone
    on UDP port 4211 and pushes them into a queue for the VAD pipeline.
    """

    def __init__(self, audio_queue: asyncio.Queue):
        self.audio_queue = audio_queue
        self.transport = None

    def connection_made(self, transport):
        self.transport = transport
        logger.info(f"[UDPAudioReceiver] Listening on port {PORT_AUDIO_UPLINK}")

    def datagram_received(self, data: bytes, addr):
        # Every packet on this port is raw PCM — no header to strip
        try:
            self.audio_queue.put_nowait(data)
        except asyncio.QueueFull:
            pass  # Drop oldest-style — consumer should drain fast enough

    def error_received(self, exc):
        logger.error(f"[UDPAudioReceiver] Error: {exc}")

async def start_udp_audio_listener(audio_queue: asyncio.Queue) -> asyncio.BaseTransport:
    """Starts the UDP listener and returns the transport for cleanup."""
    loop = asyncio.get_event_loop()
    transport, _ = await loop.create_datagram_endpoint(
        lambda: UDPAudioReceiver(audio_queue),
        local_addr=("0.0.0.0", PORT_AUDIO_UPLINK),
    )
    return transport
