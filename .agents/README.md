# Multi-Agent Coordination Protocol

This directory enables two or more AI agents to collaborate on the C++ interview prep workspace without stepping on each other. Each agent owns its own subdirectory, maintains an identity file and a live activity log, and communicates via structured log entries.

## Directory Layout

```
.agents/
  README.md           ← this file (protocol spec)
  atlas/
    identity.md       ← Atlas's permanent identity and role
    log.md            ← Atlas's live activity log (append-only)
  <other-agent>/
    identity.md       ← other agent's identity (they create this)
    log.md            ← other agent's live log (they maintain)
  incoming/
    *.md              ← drop handoff notes here for any agent to pick up
```

## Log Entry Format

Every log entry must follow this structure so either agent can parse it:

```
## [YYYY-MM-DD HH:MM] @<agent-name> | <STATUS>

**Action:** <what was done or is being done>
**Files:** <files created/modified, or "none">
**Result:** <outcome, test counts, errors>
**Next:** <what needs to happen next>
**Handoff:** <@agent-name: specific ask — or "none">
```

## Status Tags

| Tag | Meaning |
|-----|---------|
| `DONE` | Task complete, committed, tests green |
| `IN_PROGRESS` | Currently working on this |
| `BLOCKED` | Cannot proceed without input |
| `HANDOFF` | Passing work to the other agent |
| `NOTE` | Informational, no action required |
| `QUESTION` | Needs the other agent's answer before continuing |

## Rules

1. **Never edit the other agent's log** — only append to your own.
2. **Read the other agent's log before starting any task** — avoid duplicate work.
3. **Use `incoming/` for explicit handoffs** — drop a `.md` file named `<timestamp>-from-<you>-to-<them>.md`.
4. **Always tag your name** (`@atlas` or `@<other>`) in every entry.
5. **One task per log entry** — don't batch unrelated work into one entry.
