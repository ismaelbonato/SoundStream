# Feature Checklist (Implementation Status)

This checklist tracks what is currently implemented in SoundStream.

Status legend:

- ✅ **Done**
- 🟡 **Partial**
- ⬜ **Not started**

## Roadmap summary

| Tier | Status | Notes |
|------|--------|-------|
| v0.1 — Core patchbay | 🟡 Partial (near-ready) | Core graph/linking is in place; some UX/editing extras still pending. |
| v0.2 — Volume & routing | ⬜ Not started | No volume/mute/default-device/profile controls yet. |
| v0.3 — Session management | ⬜ Not started | No save/restore ruleset workflow yet. |
| v1.0 — Advanced | ⬜ Not started | No virtual cable/loopback/EQ/VU features yet. |

## Graph fundamentals 

- ✅ Live node/port/link graph rendering
- ✅ Directional ports (input/output)
- ✅ Media-type awareness (data-level + visual differentiation for ports/links with legend)
- ✅ Real-time graph updates on backend events

## Canvas interaction

- ✅ Pan / zoom / fit-to-content (`resetView` and interactive zoom/pan available)
- 🟡 Selection support (single and basic rectangle behavior exists; richer shortcuts/state UX still evolving)
- ✅ Drag-to-connect between compatible ports
- ✅ Unlink selected connections (delete/remove paths exist)
- ✅ Drag nodes with stable stacking behavior (covered by Qt interaction test)

## Editing workflows

- ⬜ Rename nodes and ports
- ⬜ Undo / redo for local graph operations
- 🟡 Link/unlink keyboard workflow (Delete is available; broader shortcut set pending)
- 🟡 Refresh/resync action (graph is live-updated; explicit user refresh action is minimal)

## Rules and persistence

- ⬜ Save/load connection rule sets
- ⬜ Activate/deactivate saved rule set
- ⬜ Exclusive mode (enforce only saved rules)
- ⬜ Pin/unpin connection persistence
- ⬜ Auto-pin and auto-disconnect options

## Filtering and grouping

- ⬜ Hide/filter nodes by patterns
- ⬜ Merge/group equivalent nodes by naming rules
- ⬜ Case-insensitive and regex-enabled matching

## UX and app behavior

- ⬜ Tray integration
- ⬜ Start minimized option
- ⬜ Helpful status notifications
- ⬜ Config persistence for layout and custom names

## Current test coverage highlights

- ✅ Core event/command policy tests (Catch2)
- ✅ Qt interaction tests for:
	- drag-to-connect
	- node movement
	- dragged-node stacking order
- ✅ Dispatcher behavior test

## Next practical targets

1. Implement explicit rename flow (node/port) and tests.
2. Add persistence foundation (graph snapshot save/load) with schema tests.
3. Add filtering/grouping options in graph UI.
4. Start Tier 2 controls: per-node volume + mute.
