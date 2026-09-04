import pytest

from audio.udp_receiver import JitterBuffer


@pytest.mark.asyncio
async def test_jitter_buffer_ordering():
    jb = JitterBuffer(ptime_ms=32, max_jitter_ms=100, packet_size_bytes=1024)

    # Push out of order, but start with 0 to initialize expected_seq
    jb.push(0, b"packet0")
    jb.push(2, b"packet2")
    jb.push(1, b"packet1")

    chunk0 = await jb.get_ordered_chunk(timeout=0.1)
    chunk1 = await jb.get_ordered_chunk(timeout=0.1)
    chunk2 = await jb.get_ordered_chunk(timeout=0.1)

    assert chunk0 == b"packet0"
    assert chunk1 == b"packet1"
    assert chunk2 == b"packet2"


@pytest.mark.asyncio
async def test_jitter_buffer_drop_late():
    jb = JitterBuffer(ptime_ms=32, max_jitter_ms=100, packet_size_bytes=1024)

    jb.push(0, b"packet0")
    jb.push(1, b"packet1")

    await jb.get_ordered_chunk(timeout=0.1)
    await jb.get_ordered_chunk(timeout=0.1)

    # 0 is now late
    jb.push(0, b"late0")

    # Should timeout because 2 never arrives
    chunk2 = await jb.get_ordered_chunk(timeout=0.01)
    assert chunk2 == b"\x00" * 1024


@pytest.mark.asyncio
async def test_jitter_buffer_silence_padding():
    jb = JitterBuffer(ptime_ms=32, max_jitter_ms=100, packet_size_bytes=1024)

    jb.push(0, b"packet0")
    # packet 1 lost
    jb.push(2, b"packet2")

    chunk0 = await jb.get_ordered_chunk(timeout=0.01)
    chunk1 = await jb.get_ordered_chunk(timeout=0.01)  # Will timeout and return silence
    chunk2 = await jb.get_ordered_chunk(timeout=0.01)

    assert chunk0 == b"packet0"
    assert chunk1 == b"\x00" * 1024
    assert chunk2 == b"packet2"
