from unittest.mock import patch

import pytest
from langchain_core.messages import AIMessage

from core.graph import AgentState, behavior_node


@pytest.mark.asyncio
async def test_behavior_node_normal_cpu():
    state: AgentState = {
        "messages": [AIMessage(content="Hello", additional_kwargs={"emotion": "HAPPY"})],
        "current_emotion": "HAPPY",
        "robot_physical_state": {"cpu_load": 50},
        "active_goal": "",
    }

    with (
        patch("core.graph.send_kinematic_intent") as mock_intent,
        patch("core.graph.broadcast_phase"),
        patch("core.graph.speak") as mock_speak,
    ):
        await behavior_node(state)

        mock_intent.assert_called_once_with("HAPPY", 0.8)
        mock_speak.assert_called_once_with("Hello")


@pytest.mark.asyncio
async def test_behavior_node_high_cpu_clamping():
    state: AgentState = {
        "messages": [AIMessage(content="I am tired", additional_kwargs={"emotion": "SAD"})],
        "current_emotion": "SAD",
        "robot_physical_state": {"cpu_load": 90},
        "active_goal": "",
    }

    with (
        patch("core.graph.send_kinematic_intent") as mock_intent,
        patch("core.graph.broadcast_phase"),
        patch("core.graph.speak") as mock_speak,
    ):
        await behavior_node(state)

        mock_intent.assert_called_once_with("SAD", 0.3)
        mock_speak.assert_called_once_with("I am tired")
