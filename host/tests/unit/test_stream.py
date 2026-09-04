import asyncio
from typing import Literal

from langchain_core.messages import HumanMessage
from langchain_ollama import ChatOllama
from pydantic import BaseModel, Field


class CognitiveOutput(BaseModel):
    emotion: Literal["HAPPY", "SAD"] = Field(...)
    response_text: str = Field(...)


async def main():
    llm = ChatOllama(model="llama3.2", format="json", temperature=0.3)
    # Don't use with_structured_output, just format="json" and tell it the schema!
    # Let's see if we can stream standard JSON from Ollama.
    prompt = (
        "Output JSON with 'emotion' and 'response_text' fields. Be very verbose in response_text."
    )

    async for chunk in llm.astream([HumanMessage(content=prompt)]):
        print(chunk.content, end="", flush=True)


asyncio.run(main())
