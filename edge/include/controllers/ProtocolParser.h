#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#include <Arduino.h>
#include <ArduinoJson.h>

enum class MessageType {
    UNKNOWN,
    INTENT,
    PHASE_UPDATE,
    EMERGENCY_STOP,
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
        
        if (doc["payload"].is<JsonObject>()) {
            result.payload = doc["payload"];
        }
        result.isValid = true;
        return result;
    }
};

#endif