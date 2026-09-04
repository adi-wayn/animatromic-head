import io
import wave

from piper.voice import PiperVoice

model_path = "models/en_GB-alan-medium.onnx"
voice = PiperVoice.load(model_path)
print(f"Sample Rate: {voice.config.sample_rate}")

wav_io = io.BytesIO()
with wave.open(wav_io, "wb") as wav_file:
    wav_file.setnchannels(1)
    wav_file.setsampwidth(2)
    wav_file.setframerate(voice.config.sample_rate)
    voice.synthesize("Hello! This is a test of the Piper TTS system.", wav_file)

audio_bytes = wav_io.getvalue()
print(f"Generated {len(audio_bytes)} bytes.")
