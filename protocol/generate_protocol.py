try:
    Import("env")
except NameError:
    pass

import json
import os

SCHEMA_PATH = os.path.join(os.path.dirname(__file__), 'schema.json')
PYTHON_OUT = os.path.join(os.path.dirname(__file__), '..', 'host', 'protocol', 'messages.py')
CPP_OUT = os.path.join(os.path.dirname(__file__), '..', 'edge', 'include', 'controllers', 'ProtocolParser.h')

def load_schema():
    with open(SCHEMA_PATH, 'r') as f:
        return json.load(f)

def generate_python(schema):
    code = [
        "from pydantic import BaseModel, Field",
        "import time",
        "import uuid",
        "",
        "class BaseMessage(BaseModel):",
        "    type: str",
        "    event_id: str = Field(default_factory=lambda: str(uuid.uuid4()))",
        "    timestamp_ms: int = Field(default_factory=lambda: int(time.time() * 1000))",
        "    payload: dict",
        ""
    ]
    
    for msg_type, fields in schema['messages'].items():
        if fields:
            class_name = "".join(word.capitalize() for word in msg_type.split('_')) + "Payload"
            code.append(f"class {class_name}(BaseModel):")
            for field_name, field_type in fields.items():
                py_type = "str" if field_type == "string" else field_type
                if py_type == "float":
                    code.append(f"    {field_name}: float = 1.0")
                else:
                    code.append(f"    {field_name}: {py_type}")
            code.append("")
    
    for msg_type, fields in schema['messages'].items():
        func_name = f"create_{msg_type.lower()}_message"
        if fields:
            args = ", ".join([f"{k}: str" if v == "string" else f"{k}: float = 1.0" for k, v in fields.items()])
            class_name = "".join(word.capitalize() for word in msg_type.split('_')) + "Payload"
            kwargs = ", ".join([f"{k}={k}" for k in fields.keys()])
            code.append(f"def {func_name}({args}) -> BaseMessage:")
            code.append(f"    return BaseMessage(")
            code.append(f"        type=\"{msg_type}\",")
            code.append(f"        payload={class_name}({kwargs}).model_dump()")
            code.append("    )")
        else:
            code.append(f"def {func_name}() -> BaseMessage:")
            code.append(f"    return BaseMessage(type=\"{msg_type}\", payload={{}})")
        code.append("")
        
    with open(PYTHON_OUT, 'w') as f:
        f.write("\n".join(code))

def generate_cpp(schema):
    code = [
        "#ifndef PROTOCOL_PARSER_H",
        "#define PROTOCOL_PARSER_H",
        "",
        "#include <Arduino.h>",
        "#include <ArduinoJson.h>",
        "",
        "enum class MessageType {",
        "    UNKNOWN,"
    ]
    
    for msg_type in schema['messages'].keys():
        code.append(f"    {msg_type},")
        
    code.extend([
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
        "            Serial.printf(\"[ProtocolParser] JSON Parse Error: %s\\n\", error.c_str());",
        "            return result;",
        "        }",
        "",
        "        if (!doc[\"type\"].is<const char*>()) {",
        "            Serial.println(\"[ProtocolParser] Missing 'type' field\");",
        "            return result;",
        "        }",
        "",
        "        String typeStr = doc[\"type\"].as<String>();"
    ])
    
    for msg_type in schema['messages'].keys():
        code.append(f"        if (typeStr == \"{msg_type}\") {{ result.type = MessageType::{msg_type}; }}")
        
    code.extend([
        "        ",
        "        if (doc[\"payload\"].is<JsonObject>()) {",
        "            result.payload = doc[\"payload\"];",
        "        }",
        "        result.isValid = true;",
        "        return result;",
        "    }",
        "};",
        "",
        "#endif"
    ])
    
    with open(CPP_OUT, 'w') as f:
        f.write("\n".join(code))

if __name__ == "__main__":
    schema = load_schema()
    generate_python(schema)
    generate_cpp(schema)
    print("Protocol code generated successfully.")
