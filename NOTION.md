# Notion integration (progress tracking)

## Status

NOT yet connected. Set up the Notion MCP server in Claude Code when ready
(standard MCP config — follow Notion's MCP docs). Do not connect until I say so.

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
