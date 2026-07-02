# MCP Configurations & Skills Mappings

This document details the active Model Context Protocol (MCP) servers and tools integrated into the agent workspace, and guidelines for creating custom Javascript/Python helper skills.

## 1. Available MCP Servers
The following MCP servers are configured in the current workspace:
* **StitchMCP:** Design, wireframe, and layout mapping system.
* **chrome-devtools-mcp:** Browser automation, console analysis, page interaction.
* **drawio:** Diagrams and layout generation from XML, CSV, or Mermaid.
* **linear-mcp-server:** Issue tracking, project attachments, and milestone planning.

## 2. PlatformIO Console & Telemetry Scripting (Future Extension)
For debugging the ESP32 animatronic head telemetry:
* **Serial Monitor Automation:** We can implement a custom Python script (executed inside the PlatformIO environment or terminal) that reads Serial logs from the ESP32 (via USB port `/dev/cu.usbserial-*`) and plots angle movements or log states to verify the servo sweeps.
* **MCP Integration:** We can leverage standard command execution tools to trigger `pio device monitor` or custom python plotting utilities.
