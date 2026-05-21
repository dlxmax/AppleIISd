# Ideas / parking lot

Unscheduled work to come back to. Newest first.

## ProDOS 2.5 swap helper (2026-05-21)

**Motivation.** Several catalog entries
(see `docs/curated-collections-prodos-2.5.md`) ship with vintage
ProDOS at the partition root (a2-dev = 2.4.x, MECC `.2mg` ≈ 2.0.3,
many others). Users who want a single ProDOS 2.5 install across all
partitions currently have to do a manual file swap per partition.

**Sketch.** A tiny helper that takes a `.po` and replaces its root
`PRODOS` with a known-good 2.5 build:

```sh
tools/upgrade-prodos.sh image.po
```

Mechanism: `cp2 copy` (CiderPress II) for the in-image swap. ~20
lines of shell.

**Open decisions.**

1. **Where the 2.5 `PRODOS` comes from**
   - *Vendored* (`tools/prodos-2.5/PRODOS`, ~17KB tracked in git):
     self-contained, reproducible, but needs manual refresh as
     prodos8.com cuts new alphas. ProDOS 2.5 is currently a8.
   - *Fetched on demand* from prodos8.com, cached under
     `~/.cache/appleiisd/`: always tracks upstream, adds network
     dep, URL could change.
   - Leaning vendored: rest of this repo is already self-contained,
     and reproducibility beats tracking unstable alphas. Bump
     intentionally.

2. **Standalone vs integrated**
   - Standalone `tools/upgrade-prodos.sh image.po` (in-place edit
     of a `.po`).
   - Or a `--upgrade-prodos` flag on the future `tools/make-card.sh`
     (the dd-wrapper discussed in `docs/sd-card-setup.md`) so the
     swap happens automatically as part of writing the card.
   - Probably both: helper as the primitive, flag as the
     convenience.

**Status.** Not built. Waiting on user decision.
