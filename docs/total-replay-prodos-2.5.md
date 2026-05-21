# Total Replay (4cade) on ProDOS 2.5 with AppleIISd

ProDOS 2.5 changes the SmartPort unit-number byte from the legacy
`DSSS0000` (2 drives) to `DSSS00XY` (8 drives, where drive 1–8 =
`X*4 + Y*2 + D + 1`). Total Replay (renamed to **4cade**,
<https://github.com/a2-4am/4cade>) talks to SmartPort drivers
directly via its own fast loader and assumes the legacy layout.
The ProDOS 2.5 release notes
(<https://prodos8.com/releases/prodos-25/>) explicitly call out
Total Replay as needing modification.

## TL;DR — workaround (no patch needed)

Install `TOTAL.REPLAY` on **partition 1 or 2** of the AppleIISd. The
DEVNUM byte ProDOS 2.5 hands the booted app for those two partitions
is bit-identical to what ProDOS 2.4 produced (`$S0` or `$80|$S0`),
and 4cade's existing code handles them correctly. The bug only fires
for partitions 3–8 (X or Y bit set).

## Status (as of 2026-05-21)

- No 4cade release supports ProDOS 2.5's new unit-number layout.
- v5.0.1 changelog says "Fixed launching from ProDOS 2.5" — that
  was an unrelated 80STORE bug, not this issue.
- No open/closed 4cade issue tracks this. Related ancestor: issue
  [#3](https://github.com/a2-4am/4cade/issues/3) — "Hangs when
  launched from GS/OS Finder on >4th SmartPort device" (2019).
- No community fork/binary patch found.

## If we ever want to fix it

Three files in `a2-4am/4cade`:

| File | Lines | What to change |
|------|-------|----------------|
| `src/prorwts2.a` | ~254–258, 3532–3540 | Stop hardcoding `$D1`; carry the full `DSSS00XY` DEVNUM through to `unrunit1`/`unrunit2`. Slot mask `AND #$70` is fine; the drive-bit assumption downstream is what breaks. |
| `src/prorwts2.a` | ~3629 | Convert DEVNUM → SmartPort device index per the 2.5 spec: `ASL / AND #7 / ADC #1`, then subtract 1 for zero-based use. |
| `src/prodos.impl.lc2.a` | ~522–538 | The LC RAM2 mini-ProDOS shim stores a fixed `$BF13–$BF1E` vector table per slot with no per-drive routing. Either route through real `$BF00` MLI, or teach the shim to pass the full DEVNUM byte to SmartPort STATUS/READ instead of synthesizing a legacy unit byte. |
| `src/4cade.init.a` | ~403–412 | Optional, only if Total Replay should enumerate games across multiple partitions: walk `$BF32–$BF56` (DEVLST, up to 37 entries in 2.5) and translate each entry. |

Build with Merlin32 or ACME — see `BUILDING.md` in the repo. Best
path is probably to open a well-scoped issue against `a2-4am/4cade`
pointing at the spec and these files; 4am tends to respond to good
issues and may accept a PR.

## Key references

- ProDOS 2.5 release notes — <https://prodos8.com/releases/prodos-25/>
- 4cade source — <https://github.com/a2-4am/4cade>
- 4cade changelog — <https://github.com/a2-4am/4cade/blob/main/res/history.md>
- Applefritter "ProDOS 8 drives per slot" thread —
  <https://www.applefritter.com/content/prodos-8-drives-slot>
