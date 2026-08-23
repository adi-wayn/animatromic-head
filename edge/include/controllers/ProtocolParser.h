#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#include <Arduino.h>
#include <ArduinoJson.h>

// --- Port Constants (auto-generated from schema.json) ---
constexpr uint16_t PORT_CONTROL = 4210;
constexpr uint16_t PORT_AUDIO_UPLINK = 4211;
constexpr uint16_t PORT_AUDIO_DOWNLINK = 4212;
constexpr uint16_t PORT_TELEMETRY = 4213;

constexpr uint32_t AUDIO_SAMPLE_RATE_HZ = 16000;
constexpr uint8_t AUDIO_BIT_DEPTH = 16;
constexpr uint8_t AUDIO_CHANNELS = 1;
constexpr uint16_t AUDIO_CHUNK_SIZE_BYTES = 2048;
constexpr uint16_t AUDIO_HEADER_SIZE_BYTES = 6;

enum class MessageType {
    UNKNOWN,
    INTENT,
    PHASE_UPDATE,
    EMERGENCY_STOP,
    TTS_COMPLETE,
    TELEMETRY,
};

struct ParsedMessage {
    MessageType type;
    JsonDocument payload;
    bool isValid;
};

class ProtocolParser {
public:
    static ParsedMessage parse(const String& json) {
        ParsedMessage result;
        result.type = MessageType::UNKNOWN;
        result.isValid = false;

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, json);
        if (error) {
            Serial.printf("[ProtocolParser] JSON Parse Error: %s\n", error.c_str());
            return result;
        }

        if (!doc["type"].is<const char*>()) {
            Serial.println("[ProtocolParser] Missing 'type' field");
            return result;
        }

        String typeStr = doc["type"].as<String>();
        if (typeStr == "INTENT") { result.type = MessageType::INTENT; }
        if (typeStr == "PHASE_UPDATE") { result.type = MessageType::PHASE_UPDATE; }
        if (typeStr == "EMERGENCY_STOP") { result.type = MessageType::EMERGENCY_STOP; }
        if (typeStr == "TTS_COMPLETE") { result.type = MessageType::TTS_COMPLETE; }
        if (typeStr == "TELEMETRY") { result.type = MessageType::TELEMETRY; }
        
        if (doc["payload"].is<JsonObject>()) {
            result.payload = doc["payload"];
        }
        result.isValid = true;
        return result;
    }
};

#endif