# Animatronic Head Platform - Agentic Entry Point

**ATTENTION ALL AI AGENTS:** This file is your primary memory and system prompt for this repository. Read it carefully at the start of every session.

## 1. Context Acquisition (The "What" and the "How")
Before writing any code or proposing solutions, you MUST read the following foundational documents:
*   **WHAT to do:** Read `docs/requirements/AnimatronicHead_SRS.md`
*   **HOW to do it:** Read `docs/design/AnimatronicHead_SDD.md`
*   **WHERE we are:** Read `docs/Implementation_Roadmap.md` to see the current phase and pending tasks.
*   **CONSTRAINTS:** Read `.agent/rules.md` and `.agent/hardware_context.md`. You must never violate these constraints.

## 2. Methodology & Decision Making (Spec-Driven Development)
We operate under a strict **Agentic Agile Methodology**:
*   **No Blind Coding:** Never jump straight to writing code based on a prompt. Always use the `/plan` command or equivalent to draft a plan first.
*   **Pros and Cons:** When faced with an architectural decision or library choice, present the pros and cons of the available options to the user.
*   **User Authority:** The user *always* has the final word. Do not proceed with an implementation until the user explicitly approves your plan.

## 3. Agentic Tooling (Skills & MCP)
You are expected to utilize your full agentic capabilities:
*   **Skills:** If a repetitive workflow exists (e.g., standard testing, deployment), use an existing Skill. If a Skill does not exist, **create a new Skill** to automate the workflow for future sessions.
*   **MCP (Model Context Protocol):** Use available MCP tools to interface with external APIs or local system processes. Install or create new MCPs if the task demands it.

## 4. Git Workflow
We do not develop directly on the `main` branch. 
*   **Branching:** Always branch off `main` for any specific task or feature (e.g., `git checkout -b feat/task-name`).
*   **Execution:** Work on the isolated branch. 
*   **Pull Request:** When the task is complete, open a Pull Request (or equivalent merge request).
*   **Merge:** Merge into `main` **ONLY** after the user explicitly reviews and accepts the work.
