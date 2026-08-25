import asyncio
from pydantic import BaseModel, Field
from typing import Literal
from langchain_ollama import ChatOllama
from langchain_core.messages import HumanMessage

class CognitiveOutput(BaseModel):
    emotion: Literal['HAPPY', 'SAD'] = Field(...)
    response_text: str = Field(...)

async def main():
    llm = ChatOllama(model="llama3.2", temperature=0.3)
    structured_llm = llm.with_structured_output(CognitiveOutput)
    
    prompt = "Tell me a very long story about a dog."
    
    prev_text = ""
    async for chunk in structured_llm.astream([HumanMessage(content=prompt)]):
        if isinstance(chunk, CognitiveOutput):
            current_text = chunk.response_text
        else:
            current_text = chunk.get("response_text", "")
            
        if current_text is None:
            continue
            
        diff = current_text[len(prev_text):]
        if diff:
            print(diff, end="", flush=True)
            prev_text = current_text
    print("\nDONE")

asyncio.run(main())
