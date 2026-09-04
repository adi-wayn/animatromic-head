import asyncio
from unittest.mock import AsyncMock, MagicMock, patch

import pytest
from langgraph.checkpoint.memory import MemorySaver

import core.graph as graph_module
from core.graph import CognitiveOutput, build_graph


@pytest.mark.asyncio
async def test_graph_routing_e2e():
    # Setup queue
    graph_module.text_queue = asyncio.Queue()

    # We will simulate a normal conversation, followed by an interrupt
    await graph_module.text_queue.put("Hello robot")
    await graph_module.text_queue.put("__INTERRUPT__")
    await graph_module.text_queue.put("__EXIT__")

    checkpointer = MemorySaver()
    graph = build_graph(checkpointer)
    config = {"configurable": {"thread_id": "1"}}

    # We mock LLM, intent, and speak. We also want to monitor routing.
    with (
        patch("core.graph.LLMManager") as mock_manager,
        patch("core.graph.send_kinematic_intent"),
        patch("core.graph.broadcast_phase"),
        patch("core.graph.speak"),
        patch("core.graph.send_emergency_stop") as mock_estop,
    ):
        # Mock structured output
        mock_llm = mock_manager.return_value.get_llm.return_value
        mock_structured = MagicMock()
        mock_structured.ainvoke = AsyncMock(
            return_value=CognitiveOutput(emotion="HAPPY", response_text="Hi")
        )
        mock_llm.with_structured_output.return_value = mock_structured

        # Track the nodes we visit
        visited_nodes = []

        # We manually step through the graph using astream so it doesn't block forever
        # when the queue is empty.

        # Initial state: None. Graph goes to listen_node, agent_node, behavior_node, listen_node
        async for event in graph.astream({"messages": []}, config):
            for node_name in event:
                visited_nodes.append(node_name)

            # If we hit listen_node and the queue is empty, wait until we've hit interrupt_node to break
            if (
                visited_nodes[-1] == "listen_node"
                and graph_module.text_queue.empty()
                and "interrupt_node" in visited_nodes
            ):
                break

        # The expected node sequence based on queue items:
        # 1. "Hello robot" -> listen_node -> agent_node -> behavior_node
        # 2. Loop back to listen_node, which gets "__INTERRUPT__"
        # 3. "__INTERRUPT__" -> interrupt_node
        # 4. Loop back to listen_node (queue empty, breaks)

        assert "listen_node" in visited_nodes
        assert "agent_node" in visited_nodes
        assert "behavior_node" in visited_nodes
        assert "interrupt_node" in visited_nodes

        # Verify emergency stop was called during the interrupt
        mock_estop.assert_called_once()
