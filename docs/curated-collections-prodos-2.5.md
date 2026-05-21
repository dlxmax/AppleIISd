# Curated collections on the AppleIISd under ProDOS 2.5

This is the working catalog of curated Apple II disk images / hard
drive collections and what it takes to run each one on an AppleIISd
with firmware **v1.3.0** (8 partitions, ProDOS 2.5 DSSS00XY
unit-number layout) plus the older DSSS0000 layout that ProDOS 2.4
and earlier still use.

For each collection we record:
- **Format** — what file(s) ship, and what conversion is needed.
- **Loader** — does its launcher use ProDOS MLI (safe) or a custom
  fast loader (potential DSSS00XY breakage)?
- **Recommended slot** — which AppleIISd partition (0–7) to put it
  on, given that partitions 1 and 2 are bit-compatible with ProDOS
  2.4's DEVNUM and partitions 3–7 require ProDOS 2.5-aware code.
- **Mods needed** — host-side conversion, ProDOS swap, source
  patches, etc.

ProDOS 2.5 changes the SmartPort unit-number byte from
`DSSS0000` (2 drives) to `DSSS00XY` (8 drives, where drive 1–8 =
`X*4 + Y*2 + D + 1`). Firmware v1.3.0 decodes both layouts in
`Firmware/src/ProDOS.s::PRODOS`, so any collection that uses ProDOS
MLI for I/O is unaffected — only collections whose launcher pokes
the SmartPort driver directly with a baked-in DSSS0000 assumption
break, and even then only on partitions 3–8 where X or Y is set.

## Slot allocation cheatsheet

| Slot | What goes here | Why |
|------|----------------|-----|
| **0** | Boot volume — your daily-driver ProDOS 2.5 install + utilities (or BurgerDisk / SmartPort / CFFA3 image) | Only autoboot target. Should contain `PRODOS` (2.5) + `BASIC.SYSTEM` + a launcher. |
| **1** | Total Replay (4cade)                   | DEVNUM bit-identical to ProDOS 2.4 `$S0`; 4cade fast loader works correctly. |
| **2** | Total Replay II: Instant Replay         | DEVNUM bit-identical to ProDOS 2.4 `$80\|$S0`; same engine, same fix. |
| **3** | Wizard Replay / Wizardry Replay 2.0     | Pure ProDOS MLI — works on any partition. Parked here to keep p1/p2 for 4cade. |
| **4** | apple-2.com collection of choice (e.g. BurgerDisk, SmartPort, etc.) | MLI launcher — works on any partition. |
| **5** | MECC Collection (or other educational)  | Standard ProDOS — works on any partition. Convert `.2mg` → `.po` first. |
| **6** | a2-dev (Merlin / Copy II Plus / Dazzle Draw / sample source) | apple-2.com's developer image — bundles Merlin, paint tools, ProDOS 2.4.2/2.4.3 system disks. Swap in PRODOS 2.5 at root. |
| **7** | Blank / scratch volume                  | Format with `cp2 create-disk-image` or in-Apple FILER. |

This is a suggestion, not a constraint. The only **hard** rule:
**4cade-based collections must go on partition 1 or 2.** Everything
else is free placement.

## Catalog

### Total Replay (action games, ~480)

- **Source:** [a2-4am/4cade](https://github.com/a2-4am/4cade) → 32MB
  `.po` from latest release.
- **Format:** Already a 32MB `.po`. dd straight onto a partition.
- **Loader:** Custom ProRWTS2 fast loader bypassing ProDOS MLI.
- **Recommended slot:** **1 or 2** (mandatory).
- **Mods needed:** None for the partition-1/2 workaround. Source
  patches required to use partition 3–8 (see "If we ever want to
  fix 4cade" below).

### Total Replay II: Instant Replay (sports/board, ~47)

- **Source:** [a2-4am/4sports](https://github.com/a2-4am/4sports) →
  32MB `.po`.
- **Format:** Already a 32MB `.po`.
- **Loader:** Same 4cade engine — confirmed: `4cade.a`,
  `prorwts2.a`, `glue.prorwts2.a`, `prodos.impl.lc2.a`.
- **Recommended slot:** **1 or 2** (mandatory). If Total Replay is
  on p1, put Instant Replay on p2.
- **Mods needed:** None for the workaround. Same upstream fix would
  apply to both.

### Wizard Replay / Wizardry Replay 2.0 (Wizardry 1–3 + Wizimore)

- **Source:** [a2-4am/wizard-replay](https://github.com/a2-4am/wizard-replay).
- **Format:** ProDOS volume; check release for size/format.
- **Loader:** Pure ProDOS MLI (`glue.mli.a` → `jsr $BF00`,
  CMD_OPEN `$03C8`, CMD_READ `$04CA`, etc.). No `prorwts2.a` in
  the source tree.
- **Recommended slot:** Any (suggested p3 to reserve p1/p2 for
  4cade).
- **Mods needed:** None.

### apple-2.com 32MB collections

The collections at <https://www.apple-2.com/> (BurgerDisk, XDrive,
XDrive2C, CFFA-3000, SmartPort, "Games" by San Inc/qkumba, Hyper,
SPSD, a2-dev) all share the same shape:

```
PRODOS              (stock)
*.SYSTEM            (small launcher, ~5 blocks — Henderson/Ferrie family)
BASIC.SYSTEM
GAMES/              (subdirs of standalone ProDOS programs)
UTILS/
```

- **Source:** apple-2.com download pages; each ships as a `.po`
  matching the 32MB partition size.
- **Format:** Already 32MB `.po`.
- **Loader:** Each `*.SYSTEM` launcher (`BURGER.SYSTEM`,
  `GAMES.SYSTEM`, `XDR2C.SYSTEM`, `CFFA3.SYSTEM`, `SMART.SYSTEM`,
  `SPSD.SYSTEM`, etc.) goes through ProDOS MLI to enumerate and
  launch.
- **Recommended slot:** Any (suggested p4).
- **Mods needed:** None for the launcher. **Caveat:** individual
  games *inside* a collection that ship their own SmartPort fast
  loader (Total-Replay-style direct poke) would still hit the
  DSSS00XY bug on partitions 3–8. The launcher is fine; specific
  games may not be. Signature: a game that misbehaves on p3–p8 but
  works on p1–p2.

### Apple II Development (a2-dev, apple-2.com)

Same shape as the rest of the apple-2.com family but called out
separately because the contents are dev-oriented and it ships
older ProDOS that a 2.5 user will want to replace.

- **Source:** <https://www.apple-2.com/> → a2-dev 32MB image.
  Updated 28-Oct-2025. File list:
  <https://www.apple-2.com/vintage-dl/text/a2-dev-list.txt>.
- **Format:** 32MB `.po`. dd straight onto a partition.
- **Loader:** `A2DEV.SYSTEM` (Henderson/Ferrie launcher) + stock
  `PRODOS` + `BASIC.SYSTEM` + `NSCLK.SYSTEM` (No-Slot Clock
  driver, deletable). Standard MLI throughout.
- **Recommended slot:** Any (suggested p6 as your dev/scratch
  partition, alongside or instead of a blank).
- **Contents of note:**
  - `MERLIN.ASM/` — Merlin Assembler v2.58 (the canonical
    on-Apple cross-assembler).
  - `GFX.EDITORS/DAZZLE.DRAW/` — Dazzle Draw double-hi-res paint.
  - `GFX.EDITORS/PAINT816/` — 8/16 Paint (Z816 SHR-capable).
  - `UTILITIES/V84/` and `V91/` — Copy II Plus v8.4 and v9.1,
    plus BitCopy.
  - `UTILITIES/PRODOS242/` and `PRODOS243/` — full ProDOS 2.4.2
    and 2.4.3 system disks (Bitsy Boot, BlockWarden, CatDoctor,
    FastDsk, MAKE.SMALL.P8, MR.FIXIT.Y2K, UNSHRINK, ADTPRO 2.0.2).
  - `SOURCE.CODE/` — small Merlin source samples
    (`DISPLAY.DHR.S`, `GBASCALC.S`, `ULTIMA.SND.S`, `ZEEP.SND.S`).
  - `IMAGES.DHR/ABST00–ABST23` + `ABST23.SYSTEM` — 24 sample DHR
    abstract images with a viewer.
- **Mods needed:**
  - **ProDOS swap if you want 2.5 throughout.** The image ships
    PRODOS dated 30-DEC-23 (a 2.4.x build) at root and bundles
    2.4.2 / 2.4.3 system disks. To boot this partition under
    ProDOS 2.5, replace the root `PRODOS` file with the 2.5
    build (via `cp2`, Copy II Plus, or in-Apple). The 2.4.x
    bundles inside `UTILITIES/` can stay — they're convenient for
    testing software against older ProDOS.
  - None for the dev tools themselves — they're stock and
    MLI-clean.

### French Touch demos (individual floppy releases)

- **Source:** <http://fr3nch.t0uch.free.fr/> ("FRENCH TOUCH VAULT").
  ~28 releases from 2014–2024: demos, intros, musidisks,
  animadisks, slideshows. Group members: Grouik (founder) and
  Fenarinarsa. Each release is its own per-disk page on the site.
- **Format:** Floppy disk images — typically 140KB DOS 3.3 / ProDOS
  5.25" or 800KB Unidisk 3.5", **not** 32MB ProDOS volumes. Most
  recent release **DIX** (Nov 2024) is an Anthology that packs
  many of their prods onto a single 800K floppy.
- **Loader:** Mixed and per-release. Many demos boot direct from
  floppy and don't touch mass storage at all. FT also publishes a
  custom **"FAST LOADER"** (Mar 2021) which is exactly the kind of
  direct SmartPort/Disk II poke that risks DSSS00XY breakage if
  invoked from an AppleIISd partition 3–8.
- **Recommended slot:** N/A for the canonical experience.
  - **Best path: Floppy Emu** or in-emulator (AppleWin / Virtual II
    / MAME). Boot each release as its own floppy image.
  - **AppleIISd path:** copy demo contents into a 32MB ProDOS
    partition and launch as ProDOS programs from a menu. Many
    demos will run; some that hardcode floppy I/O won't. If using
    p3–p8 and a demo's loader pokes SmartPort directly, it'll hit
    the same DSSS00XY bug — fall back to p1/p2.
- **Mods needed:**
  - Per-release: check whether the disk uses ProDOS or DOS 3.3.
    DOS 3.3 disks need conversion to ProDOS file format before
    they can live inside a ProDOS partition.
  - Hardware: many FT demos require Mockingboard (sound) and
    target 50Hz PAL timing; some have NTSC variants in the same
    release. Check each release page.
- **Notable releases:** DIX (Anthology, 2024); Plasmagoria;
  Crazy Cycles I/II; MAD EFFECT #1–3; Pure Noise (Mockingboard
  music). Full index at the site.

### MECC Collection (8-bit educational)

- **Source:**
  <https://www.whatisthe2gs.apple2.org.za/mecc-collection-run-classic-eduware-titles-from-a-hard-drive-as-well-as-gs-os/index.html>
  → "MECC Vol1.2mg" + "MECC Vol2.2mg" (~6.3MB each, ~12.6MB total).
  Credited to Marco Verpelli; menu by MECC themselves (1994-95).
- **Format:** `.2mg` (64-byte header + raw ProDOS data).
- **Loader:** Standard ProDOS 8 + MECC's own menu utility. Up to
  158 programs per menu. Also launchable from Bitsy Bye or GS/OS
  System 6 Finder.
- **Recommended slot:** Any (suggested p5).
- **Mods needed:**
  1. Strip the .2mg header to get a raw .po:
     ```
     dd if="MECC Vol1.2mg" of=mecc-vol1.po bs=64 skip=1
     ```
  2. The .2mg ships with vintage ProDOS (~2.0.3). If you want to
     boot the partition directly under ProDOS 2.5, swap in the
     modern `PRODOS` file (via `cp2`, Copy II Plus, or ProSEL).
     If you launch the MECC menu from a different boot partition,
     this is unnecessary.
  3. Each volume is only ~6.3MB inside a 32MB partition — wasteful
     but harmless. Alternative: merge Vol1+Vol2 contents into a
     single partition that also hosts other educational software.

## Why the loader architecture matters

4cade uses **ProRWTS2**, its own fast loader that pokes the
SmartPort driver directly to skip ProDOS overhead. That loader
hardcodes the legacy `DSSS0000` interpretation of DEVNUM, so when
ProDOS 2.5 hands it a unit byte with X or Y set, it computes the
wrong drive.

Everything else in the catalog (Wizard Replay, the Henderson/Ferrie
launchers, MECC's menu) calls `$BF00` for every file operation and
never touches DEVNUM itself. ProDOS does the unit-→-driver
translation and our firmware
(`Firmware/src/ProDOS.s::PRODOS` → `SMBASE` decode) maps the
DSSS00XY byte back to a partition index. The whole chain stays
correct.

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
- apple-2.com collections index — <https://www.apple-2.com/>
- MECC Collection — <https://www.whatisthe2gs.apple2.org.za/mecc-collection-run-classic-eduware-titles-from-a-hard-drive-as-well-as-gs-os/index.html>
- French Touch Vault (demoscene releases) — <http://fr3nch.t0uch.free.fr/>
- 4cade changelog — <https://github.com/a2-4am/4cade/blob/main/res/history.md>
- Applefritter "ProDOS 8 drives per slot" thread —
  <https://www.applefritter.com/content/prodos-8-drives-slot>
