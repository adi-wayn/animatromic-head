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

## 4. Git Workflow & The 16-Step Agentic Feature Workflow
We do NOT develop directly on the `main` branch. **THE `main` BRANCH IS STRICTLY READ-ONLY!!!**
You MUST use the `agentic-task-workflow` skill and follow its 16 steps precisely:
1. **Create Branch:** Create a new branch (e.g., `feature/task-name`).
2. **Checkout:** Branch off `main`.
3. **Isolation:** Work ONLY on this branch.
4. **Context Gathering:** Read the `.agent/` files, SRS, SDD.
5. **Roadmap Alignment:** Check `docs/Implementation_Roadmap.md`.
6. **Planning:** Create a `/plan` artifact, outline pros/cons.
7. **Verification against Specs:** Ensure alignment with SRS/SDD.
8. **Step-by-Step Implementation:** Implement the plan, keeping the user in the loop.
9. **Post-Implementation Verification:** Check alignment again. No scope drift.
10. **Documentation Update:** Update the `walkthrough.md` and roadmap.
11. **Knowledge Synchronization:** Extract lessons and update ALL institutional knowledge (`.agent/rules.md`, `.agent/hardware_context.md`, `AGENT.md`, SDD, SRS, etc.). Future agents MUST be fully updated.
12. **Pull Request:** Open a PR / present changes for user acceptance.
13. **Merge:** After user approval, merge into `main`.
14. **Cleanup:** Delete the feature branch.
15. **Return:** Return to `main`.
16. **Close:** Inform the user.

**NEVER skip these steps. The `.agent/` files and `AGENT.md` are your primary memory and MUST be kept updated.**
