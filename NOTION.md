# Notion integration (progress tracking)

## Status

CONNECTED (2026-07-25) via the Notion MCP server, workspace "My Notion".

## Where it lives

- **Hobby OS** (parent page) — https://app.notion.com/p/3a845e1f36a981fa9a1ef0b91527abbc
  - **Kernel Tasks** (database) — https://app.notion.com/p/95652f8636584173a2c33e8cf2d81b43
    - Views: `Board` (Kanban, grouped by Status) and `By milestone` (table, grouped by Milestone)
  - **Reading log** — https://app.notion.com/p/3a845e1f36a9818ea1c4d7ea8c968c84
  - **Bug graveyard** — https://app.notion.com/p/3a845e1f36a981c1b094efa138e7eef7

Schema: Name, Status (Backlog/Reading/Building/Blocked/Done), Type
(Reading/Implementation/Debugging), Milestone (1–7), Resource (URL),
Notes (text), Parent item ↔ Sub-items (self-relation).

All seven milestones exist as parent items. Milestones 1, 2, 4 and 7 are broken
into sub-items; 3, 5 and 6 are stubs to fill in when they get close.

Milestone 7 ("7 - DOOM", added 2026-08-08) is the north-star capstone — see the
Goal section of CLAUDE.md. It comes after 1–6, not instead of them.

## Division of responsibility

- CLAUDE.md = stable project definition. Decisions, conventions, stack,
  how-to-work-with-me rules. Changes rarely. Source of truth for "what/how."
- Notion = living progress tracker. Changes constantly. Source of truth
  for "where am I / what did I learn."
  Keep the roadmap checklist in Notion once connected; CLAUDE.md keeps the
  roadmap only as high-level context, not as the tracked checklist.

## Intended Notion structure (build when connected)

Use a Notion DATABASE (not a flat checklist) so it works like a Jira board.

- Task database with these views:
  - Board view (Kanban): columns Backlog → Reading → Building → Blocked → Done
  - By-milestone view: grouped by milestone for roadmap overview
- Each milestone = a parent item; break into SUB-ITEMS (subtasks), e.g.:
  Milestone 2 (interrupts):
  📖 Read OSDev IDT page
  📖 Read OSDev PIC/APIC pages
  🔨 Set up IDT structure
  🔨 Write exception handler stubs
  🔨 Wire timer + keyboard IRQs
  ✅ Verify: keypress → serial output
- Properties per task:
  - Type: Reading / Implementation / Debugging
  - Resource: link to the OSDev page / blog / doc
  - Notes: what I learned or what broke (my words)
- Keep "read first → build after" as parent→subtask dependency chains
- Separate pages (not the task DB): Reading log, Bug graveyard

## When connected, how I want it used

- Update the checklist when I tell you a milestone is done
- Help me capture per-milestone notes and bug write-ups in my own words
  (don't write them for me — prompt me, then record what I say)
