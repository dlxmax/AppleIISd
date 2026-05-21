# 4am Replay collections on ProDOS 2.5 with AppleIISd

ProDOS 2.5 changes the SmartPort unit-number byte from the legacy
`DSSS0000` (2 drives) to `DSSS00XY` (8 drives, where drive 1–8 =
`X*4 + Y*2 + D + 1`). The new firmware in this repo (v1.3.0)
decodes both layouts. The question is whether 4am's various
"Replay" front-ends do.

## Verdict per collection (checked 2026-05-21)

| Collection | Repo | Loader | ProDOS 2.5 status |
|------------|------|--------|--------------------|
| **Total Replay** (action games, ~480) | [a2-4am/4cade](https://github.com/a2-4am/4cade) | Custom (ProRWTS2) bypassing MLI | **Buggy on partitions 3–8.** Use partition 1 or 2. |
| **Total Replay II: Instant Replay** (sports/board games, ~47) | [a2-4am/4sports](https://github.com/a2-4am/4sports) | Same 4cade engine (`4cade.a`, `prorwts2.a`, `glue.prorwts2.a`, `prodos.impl.lc2.a`) | **Same bug, same workaround.** Use partition 1 or 2. |
| **Wizard Replay / Wizardry Replay 2.0** (Wizardry 1–3 + Wizimore) | [a2-4am/wizard-replay](https://github.com/a2-4am/wizard-replay) | Pure ProDOS MLI (`glue.mli.a` → `jsr $BF00`); no `prorwts2.a` in tree | **Works on any partition.** No mods needed. |

## TL;DR — workaround for the 4cade-based collections

Install `TOTAL.REPLAY` (and `TOTAL.REPLAY2` / Instant Replay) on
**partition 1 or 2** of the AppleIISd. The DEVNUM byte ProDOS 2.5
hands the booted app for those two partitions is bit-identical to
what ProDOS 2.4 produced (`$S0` or `$80|$S0`), and 4cade's existing
code handles them correctly. The bug only fires for partitions 3–8
(X or Y bit of DSSS00XY set).

Wizard Replay has no constraint — install it anywhere.

## Other curated 32MB images (apple-2.com)

The 32MB collections at <https://www.apple-2.com/> (BurgerDisk,
XDrive, XDrive2C, CFFA-3000, "Games" by San Inc/qkumba, Hyper,
SPSD, a2-dev) all share the same shape: stock `PRODOS` + a small
`*.SYSTEM` launcher (`BURGER.SYSTEM`, `GAMES.SYSTEM`,
`XDR2C.SYSTEM`, `CFFA3.SYSTEM`, etc., mostly by Daniel Henderson
and Peter Ferrie) + `BASIC.SYSTEM` + GAMES/UTILS directories of
standalone ProDOS programs. The launcher goes through ProDOS MLI
to enumerate and launch — same shape as Wizard Replay. **No
ProRWTS-style fast loader in the launcher path → works on any
AppleIISd partition under ProDOS 2.5.**

Caveat: individual games *inside* these collections that ship
their own SmartPort fast loader (a Total-Replay-style direct poke)
would still hit the DSSS00XY bug. The launcher is fine; specific
games may not be. If you find one that misbehaves on partitions
3–8 but works on 1–2, that's the signature.

## Why Wizard Replay is fine and 4cade isn't

4cade has its own fast loader (ProRWTS2) that pokes the SmartPort
driver directly to skip ProDOS overhead. That loader hardcodes the
legacy `DSSS0000` interpretation of DEVNUM, so when ProDOS 2.5
hands it a unit byte with X or Y set, it computes the wrong drive.

Wizard Replay's `glue.mli.a` calls `$BF00` for every file
operation (CMD_OPEN `$03C8`, CMD_READ `$04CA`, etc.) and never
touches `DEVNUM` itself. ProDOS does the unit-→-driver translation
and our firmware (`Firmware/src/ProDOS.s::PRODOS` → `SMBASE`
decode) maps the DSSS00XY byte back to a partition index. The
whole chain stays correct.

## Status (as of 2026-05-21)

- No 4cade release supports ProDOS 2.5's new unit-number layout.
- 4cade v5.0.1 changelog says "Fixed launching from ProDOS 2.5" —
  that was an unrelated 80STORE bug, not this issue.
- No open/closed 4cade issue tracks this. Related ancestor: issue
  [#3](https://github.com/a2-4am/4cade/issues/3) — "Hangs when
  launched from GS/OS Finder on >4th SmartPort device" (2019).
- No community fork/binary patch found.
- Total Replay II (`4sports`) inherits the bug since it ships the
  same engine; no separate tracking issue there either.

## If we ever want to fix 4cade

Three files in `a2-4am/4cade` (identical layout in `a2-4am/4sports`):

| File | Lines | What to change |
|------|-------|----------------|
| `src/prorwts2.a` | ~254–258, 3532–3540 | Stop hardcoding `$D1`; carry the full `DSSS00XY` DEVNUM through to `unrunit1`/`unrunit2`. Slot mask `AND #$70` is fine; the drive-bit assumption downstream is what breaks. |
| `src/prorwts2.a` | ~3629 | Convert DEVNUM → SmartPort device index per the 2.5 spec: `ASL / AND #7 / ADC #1`, then subtract 1 for zero-based use. |
| `src/prodos.impl.lc2.a` | ~522–538 | The LC RAM2 mini-ProDOS shim stores a fixed `$BF13–$BF1E` vector table per slot with no per-drive routing. Either route through real `$BF00` MLI, or teach the shim to pass the full DEVNUM byte to SmartPort STATUS/READ instead of synthesizing a legacy unit byte. |
| `src/4cade.init.a` | ~403–412 | Optional, only if Total Replay should enumerate games across multiple partitions: walk `$BF32–$BF56` (DEVLST, up to 37 entries in 2.5) and translate each entry. |

Build with Merlin32 or ACME — see `BUILDING.md` in each repo.
Best path is probably to open a well-scoped issue against
`a2-4am/4cade` pointing at the spec and these files; 4am tends to
respond to good issues and may accept a PR. A fix accepted upstream
would propagate to `4sports` (Instant Replay) on its next refresh.

## Key references

- ProDOS 2.5 release notes — <https://prodos8.com/releases/prodos-25/>
- 4cade source (Total Replay) — <https://github.com/a2-4am/4cade>
- 4sports source (Total Replay II: Instant Replay) — <https://github.com/a2-4am/4sports>
- wizard-replay source (Wizardry Replay) — <https://github.com/a2-4am/wizard-replay>
- 4cade changelog — <https://github.com/a2-4am/4cade/blob/main/res/history.md>
- Applefritter "ProDOS 8 drives per slot" thread —
  <https://www.applefritter.com/content/prodos-8-drives-slot>
