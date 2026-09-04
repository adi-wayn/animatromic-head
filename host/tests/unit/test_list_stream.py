import asyncio
from typing import Literal

from langchain_core.messages import HumanMessage
from langchain_ollama import ChatOllama
from pydantic import BaseModel, Field


class CognitiveBlock(BaseModel):
    emotion: Literal["HAPPY", "SAD", "ANGRY"] = Field(description="The emotion for this segment")
    text: str = Field(description="The spoken text for this segment")


class CognitiveOutput(BaseModel):
    blocks: list[CognitiveBlock] = Field(description="A list of sequential emotion and text blocks")


async def main():
    llm = ChatOllama(model="llama3.2", temperature=0.3)
    structured_llm = llm.with_structured_output(CognitiveOutput)

    prompt = "Tell me a short story where you start happy, then get sad, then get angry."

    async for chunk in structured_llm.astream([HumanMessage(content=prompt)]):
        print(chunk)


asyncio.run(main())
