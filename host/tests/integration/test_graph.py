import pytest
import asyncio
from langchain_core.messages import HumanMessage, AIMessage
from unittest.mock import patch, MagicMock, AsyncMock

import core.graph as graph_module
from core.graph import agent_node, AgentState, CognitiveOutput

@pytest.mark.asyncio
async def test_agent_node_structured_output():
    # Mock LLM to return our Pydantic object
    mock_llm = MagicMock()
    mock_structured_llm = MagicMock()
    
    mock_result = CognitiveOutput(emotion="SAD", response_text="I feel lonely")
    mock_structured_llm.ainvoke = AsyncMock(return_value=mock_result)
    mock_llm.with_structured_output.return_value = mock_structured_llm
    
    state: AgentState = {"messages": [HumanMessage(content="Hello")]}
    
    with patch("core.graph.LLMManager") as mock_manager:
        mock_manager.return_value.get_llm.return_value = mock_llm
        
        new_state = await agent_node(state)
        
        # Check that state was updated with Pydantic values
        result_msg = new_state["messages"][0]
        assert result_msg.content == "I feel lonely"
        assert new_state["current_emotion"] == "SAD"
