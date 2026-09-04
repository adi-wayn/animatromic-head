"""LLM Manager for configuring and loading local language models."""

import logging
import subprocess

from langchain_ollama import ChatOllama
from pydantic import BaseModel

logger = logging.getLogger(__name__)


class LLMManager:
    """
    Singleton & Factory for managing native Ollama LLM instances.
    """

    _instance = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            cls._instance._initialized = False
        return cls._instance

    def __init__(self):
        if self._initialized:
            return
        self.models_cache: dict[str, ChatOllama] = {}
        self._initialized = True

    def ensure_model_exists(self, model_name: str) -> bool:
        """
        Dynamically pull the model if it's missing from the local Ollama instance.
        """
        try:
            # Check if ollama is accessible
            result = subprocess.run(["ollama", "list"], capture_output=True, text=True, check=True)

            if model_name not in result.stdout:
                logger.info(
                    f"Model '{model_name}' not found locally. Pulling dynamically (this may take a while)..."
                )
                subprocess.run(["ollama", "pull", model_name], check=True)
                logger.info(f"Successfully pulled '{model_name}'.")
            return True
        except FileNotFoundError:
            logger.error("Ollama CLI not found! Please install Ollama natively on macOS.")
            return False
        except subprocess.CalledProcessError as e:
            logger.error(f"Failed to pull model '{model_name}': {e}")
            return False

    def get_llm(
        self,
        model_name: str,
        format_schema: type[BaseModel] | None = None,
        temperature: float = 0.0,
    ) -> ChatOllama:
        """
        Factory method to get a configured ChatOllama instance.
        """
        cache_key = (
            f"{model_name}_{format_schema.__name__ if format_schema else 'none'}_{temperature}"
        )

        if cache_key not in self.models_cache:
            self.ensure_model_exists(model_name)

            # Configure ChatOllama
            kwargs = {
                "model": model_name,
                "temperature": temperature,
                "base_url": "http://localhost:11434",
            }
            if format_schema:
                kwargs["format"] = "json"  # Ollama natively supports JSON mode

            llm = ChatOllama(**kwargs)

            # If using Pydantic schema for structured output via LangChain
            if format_schema:
                llm = llm.with_structured_output(format_schema)

            self.models_cache[cache_key] = llm

        return self.models_cache[cache_key]
