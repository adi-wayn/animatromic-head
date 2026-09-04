"""Script to generate Python and C++ protocol files from JSON schema."""

try:
    Import("env")
except NameError:
    pass

import json
import os

# Resolve the protocol/ directory. When run as a PlatformIO SCons pre-build hook,
# __file__ is not defined — fall back to locating the script relative to the
# project root (SCons always runs from the PlatformIO project directory, i.e. edge/).
try:
    _SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
except NameError:
    # Running inside SCons (PlatformIO pre-build hook).
    # CWD is the edge/ directory; protocol/ is one level up.
    _SCRIPT_DIR = os.path.abspath(os.path.join(os.getcwd(), "..", "protocol"))

SCHEMA_PATH = os.path.join(_SCRIPT_DIR, "schema.json")
PYTHON_OUT = os.path.join(_SCRIPT_DIR, "..", "host", "protocol", "messages.py")
CPP_OUT = os.path.join(
    _SCRIPT_DIR, "..", "edge", "include", "controllers", "ProtocolParser.h"
)


def load_schema():
    with open(SCHEMA_PATH, "r") as f:
        return json.load(f)


def generate_python(schema):
    code = [
        "from pydantic import BaseModel, Field",
        "import time",
        "import uuid",
        "",
    ]

    # --- Port Constants ---
    if "ports" in schema:
        code.append("# --- Port Constants (auto-generated from schema.json) ---")
        for port_name, port_value in schema["ports"].items():
            code.append(f"PORT_{port_name} = {port_value}")
        code.append("")

    # --- Audio Format Constants ---
    if "audio_format" in schema:
        code.append(
            "# --- Audio Format Constants (auto-generated from schema.json) ---"
        )
        for key, value in schema["audio_format"].items():
            code.append(f"AUDIO_{key.upper()} = {value}")
        code.append("")

    # --- BaseMessage ---
    code.extend(
        [
            "class BaseMessage(BaseModel):",
            "    type: str",
            "    event_id: str = Field(default_factory=lambda: str(uuid.uuid4()))",
            "    timestamp_ms: int = Field(default_factory=lambda: int(time.time() * 1000))",
            "    payload: dict",
            "",
        ]
    )

    # --- Payload Models ---
    for msg_type, fields in schema["messages"].items():
        if fields:
            class_name = (
                "".join(word.capitalize() for word in msg_type.split("_")) + "Payload"
            )
            code.append(f"class {class_name}(BaseModel):")
            for field_name, field_type in fields.items():
                if field_type == "string":
                    py_type = "str"
                elif field_type == "int":
                    py_type = "int"
                elif field_type == "array of floats":
                    py_type = "list[float]"
                else:
                    py_type = field_type

                if py_type == "float":
                    code.append(f"    {field_name}: float = 1.0")
                else:
                    code.append(f"    {field_name}: {py_type}")
            code.append("")

    # --- Factory Functions ---
    for msg_type, fields in schema["messages"].items():
        func_name = f"create_{msg_type.lower()}_message"
        if fields:
            args_list = []
            for k, v in fields.items():
                if v == "string":
                    args_list.append(f"{k}: str")
                elif v == "int":
                    args_list.append(f"{k}: int")
                elif v == "array of floats":
                    args_list.append(f"{k}: list[float]")
                elif v == "float":
                    args_list.append(f"{k}: float = 1.0")
                else:
                    args_list.append(f"{k}: Any")
            args = ", ".join(args_list)
            class_name = (
                "".join(word.capitalize() for word in msg_type.split("_")) + "Payload"
            )
            kwargs = ", ".join([f"{k}={k}" for k in fields.keys()])
            code.append(f"def {func_name}({args}) -> BaseMessage:")
            code.append("    return BaseMessage(")
            code.append(f'        type="{msg_type}",')
            code.append(f"        payload={class_name}({kwargs}).model_dump()")
            code.append("    )")
        else:
            code.append(f"def {func_name}() -> BaseMessage:")
            code.append(f'    return BaseMessage(type="{msg_type}", payload={{}})')
        code.append("")

    with open(PYTHON_OUT, "w") as f:
        f.write("\n".join(code))


def generate_cpp(schema):
    code = [
        "#ifndef PROTOCOL_PARSER_H",
        "#define PROTOCOL_PARSER_H",
        "",
        "#include <Arduino.h>",
        "#include <ArduinoJson.h>",
        "",
    ]

    # --- Port Constants ---
    if "ports" in schema:
        code.append("// --- Port Constants (auto-generated from schema.json) ---")
        for port_name, port_value in schema["ports"].items():
            code.append(f"constexpr uint16_t PORT_{port_name} = {port_value};")
        code.append("")

    # --- Audio Format Constants ---
    if "audio_format" in schema:
        code.append(
            "// --- Audio Format Constants (auto-generated from schema.json) ---"
        )
        type_map = {
            "sample_rate_hz": "uint32_t",
            "bit_depth": "uint8_t",
            "channels": "uint8_t",
            "chunk_size_bytes": "uint16_t",
        }
        for key, value in schema["audio_format"].items():
            c_type = type_map.get(key, "uint16_t")
            code.append(f"constexpr {c_type} AUDIO_{key.upper()} = {value};")
        code.append("")

    # --- MessageType Enum ---
    code.append("enum class MessageType {")
    code.append("    UNKNOWN,")

    for msg_type in schema["messages"].keys():
        code.append(f"    {msg_type},")

    code.extend(
        [
            "};",
            "",
            "struct ParsedMessage {",
            "    MessageType type;",
            "    JsonDocument payload;",
            "    bool isValid;",
            "};",
            "",
            "class ProtocolParser {",
            "public:",
            "    static ParsedMessage parse(const String& json) {",
            "        ParsedMessage result;",
            "        result.type = MessageType::UNKNOWN;",
            "        result.isValid = false;",
            "",
            "        JsonDocument doc;",
            "        DeserializationError error = deserializeJson(doc, json);",
            "        if (error) {",
            '            Serial.printf("[ProtocolParser] JSON Parse Error: %s\\n", error.c_str());',
            "            return result;",
            "        }",
            "",
            '        if (!doc["type"].is<const char*>()) {',
            "            Serial.println(\"[ProtocolParser] Missing 'type' field\");",
            "            return result;",
            "        }",
            "",
            '        String typeStr = doc["type"].as<String>();',
        ]
    )

    for msg_type in schema["messages"].keys():
        code.append(
            f'        if (typeStr == "{msg_type}") {{ result.type = MessageType::{msg_type}; }}'
        )

    code.extend(
        [
            "        ",
            '        if (doc["payload"].is<JsonObject>()) {',
            '            result.payload = doc["payload"];',
            "        }",
            "        result.isValid = true;",
            "        return result;",
            "    }",
            "};",
            "",
            "#endif",
        ]
    )

    with open(CPP_OUT, "w") as f:
        f.write("\n".join(code))


if __name__ == "__main__":
    schema = load_schema()
    generate_python(schema)
    generate_cpp(schema)
    print("Protocol code generated successfully.")
