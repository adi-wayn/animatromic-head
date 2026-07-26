import asyncio
import struct
from loguru import logger
from protocol.messages import PORT_AUDIO_UPLINK, AUDIO_CHUNK_SIZE_BYTES

class JitterBuffer:
    def __init__(self, ptime_ms=32, max_jitter_ms=100, packet_size_bytes=AUDIO_CHUNK_SIZE_BYTES):
        self.buffer = {}
        self.expected_seq = None
        self.highest_seq = None
        
        self.max_packets = max_jitter_ms // ptime_ms
        self.packet_size_bytes = packet_size_bytes
        
        self.packet_ready = asyncio.Event()

    def _unwrap_sequence(self, seq_16):
        if self.highest_seq is None:
            return seq_16
            
        MAX_16, HALF_MAX_16 = 1 << 16, 1 << 15
        diff = seq_16 - (self.highest_seq % MAX_16)
        
        if diff < -HALF_MAX_16:
            diff += MAX_16
        elif diff > HALF_MAX_16:
            diff -= MAX_16
            
        return self.highest_seq + diff

    def push(self, seq_16, payload):
        abs_seq = self._unwrap_sequence(seq_16)
        
        if self.highest_seq is None:
            self.highest_seq = abs_seq
            self.expected_seq = abs_seq
        else:
            self.highest_seq = max(self.highest_seq, abs_seq)

        if abs_seq < self.expected_seq:
            logger.debug(f"Dropped late packet: {abs_seq}")
            return
            
        if abs_seq > self.expected_seq + self.max_packets * 2:
            logger.warning(f"Dropped future packet (buffer overflow guard): {abs_seq}")
            return

        self.buffer[abs_seq] = payload
        if abs_seq == self.expected_seq:
            self.packet_ready.set()

    async def get_ordered_chunk(self, timeout=0.060):
        try:
            if self.expected_seq not in self.buffer:
                await asyncio.wait_for(self.packet_ready.wait(), timeout=timeout)
                
            payload = self.buffer.pop(self.expected_seq)
        except asyncio.TimeoutError:
            logger.debug(f"Packet lost (seq {self.expected_seq}), generating silence.")
            payload = b'\x00' * self.packet_size_bytes
            
        self.expected_seq += 1
        self.packet_ready.clear()
        
        if self.expected_seq in self.buffer:
            self.packet_ready.set()
            
        return payload


class UDPAudioReceiver(asyncio.DatagramProtocol):
    """
    Receives framed PCM audio chunks from the ESP32 INMP441 microphone
    on UDP port 4211 and pushes them into a jitter buffer for reordering.
    """

    def __init__(self, jitter_buffer: JitterBuffer):
        self.jitter_buffer = jitter_buffer
        self.transport = None

    def connection_made(self, transport):
        self.transport = transport
        logger.info(f"[UDPAudioReceiver] Listening on port {PORT_AUDIO_UPLINK}")

    def datagram_received(self, data: bytes, addr):
        # 6-byte header minimum
        if len(data) <= 6:
            return
        
        # 6-byte header: seq_num (uint16), timestamp (uint32)
        seq_16 = struct.unpack(">H", data[:2])[0]
        payload = data[6:]
        
        self.jitter_buffer.push(seq_16, payload)

    def error_received(self, exc):
        logger.error(f"[UDPAudioReceiver] Error: {exc}")


async def stt_consumer_task(jitter_buffer: JitterBuffer, audio_queue: asyncio.Queue):
    """Consumer loop that constantly feeds the VAD pipeline with ordered audio chunks."""
    # Pre-buffering phase
    while len(jitter_buffer.buffer) < 3:
        await asyncio.sleep(0.01)
        
    while True:
        # Wait up to 60ms for the exact packet, else inject silence
        audio_chunk = await jitter_buffer.get_ordered_chunk(timeout=0.060)
        try:
            audio_queue.put_nowait(audio_chunk)
        except asyncio.QueueFull:
            pass


async def start_udp_audio_listener(audio_queue: asyncio.Queue) -> asyncio.BaseTransport:
    """Starts the UDP listener, JitterBuffer consumer, and returns the transport for cleanup."""
    loop = asyncio.get_event_loop()
    
    # Each chunk is 1024 bytes at 16kHz (mono, 16-bit) -> 1024 / (16000 * 2) = 32ms ptime
    jitter_buffer = JitterBuffer(ptime_ms=32, max_jitter_ms=100, packet_size_bytes=AUDIO_CHUNK_SIZE_BYTES)
    
    transport, _ = await loop.create_datagram_endpoint(
        lambda: UDPAudioReceiver(jitter_buffer),
        local_addr=("0.0.0.0", PORT_AUDIO_UPLINK),
    )
    
    asyncio.create_task(stt_consumer_task(jitter_buffer, audio_queue))
    
    return transport
