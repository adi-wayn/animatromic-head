import pytest
import asyncio
from langchain_core.messages import HumanMessage, AIMessage
from unittest.mock import patch, MagicMock, AsyncMock

import core.graph as graph_module
from core.graph import agent_node, AgentState, route_after_agent

@pytest.mark.asyncio
async def test_agent_node_parses_json():
    # Mock LLM to return our custom JSON
    mock_llm = MagicMock()
    mock_response = AIMessage(content='{"speak": "I am testing", "intent": "JAW", "intensity": 0.5}')
    mock_llm.ainvoke = AsyncMock(return_value=mock_response)
    
    state: AgentState = {"messages": [HumanMessage(content="Hello")]}
    
    with patch("core.graph.LLMManager") as mock_manager:
        mock_manager.return_value.get_llm.return_value = mock_llm
        
        new_state = await agent_node(state)
        
        # Check that tool calls were parsed correctly
        result_msg = new_state["messages"][0]
        assert hasattr(result_msg, "tool_calls")
        
        tool_calls = result_msg.tool_calls
        assert len(tool_calls) == 2
        assert tool_calls[0]["name"] == "speak"
        assert tool_calls[0]["args"]["text"] == "I am testing"
        assert tool_calls[1]["name"] == "send_kinematic_intent"
        assert tool_calls[1]["args"]["emotion_primary"] == "JAW"
        assert tool_calls[1]["args"]["intensity_level"] == 0.5

def test_route_after_agent():
    # Test routing when tools are used
    msg_with_tools = AIMessage(content="{}", tool_calls=[{"name": "speak", "args": {"text": "hi"}, "id": "1"}])
    state_tools: AgentState = {"messages": [msg_with_tools]}
    
    with patch("core.graph.broadcast_phase"):
        assert route_after_agent(state_tools) == "action_node"
        
    # Test routing when no tools are used
    msg_no_tools = AIMessage(content="{}")
    state_no_tools: AgentState = {"messages": [msg_no_tools]}
    assert route_after_agent(state_no_tools) == "listen_node"
