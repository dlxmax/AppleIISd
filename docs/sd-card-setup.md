# Setting up the SD card for AppleIISd

## How the layout works

The firmware treats the SD card as a raw block device — **no FAT,
no partition table, no header**. It slices the card into up to
**8 × 32 MiB ProDOS volumes laid end-to-end** by byte offset:

| Partition | Byte offset    | Size                          |
|----------:|---------------:|-------------------------------|
| 0 (boot)  | `0x00000000`   | 65535 blocks (33,553,920 B)   |
| 1         | `0x02000000`   | 65535 blocks                  |
| 2         | `0x04000000`   | 65535 blocks                  |
| 3         | `0x06000000`   | 65535 blocks                  |
| 4         | `0x08000000`   | 65535 blocks                  |
| 5         | `0x0A000000`   | 65535 blocks                  |
| 6         | `0x0C000000`   | 65535 blocks                  |
| 7         | `0x0E000000`   | 65535 blocks                  |

Stride is exactly **32 MiB = 33,554,432 B** (8 × that = 256 MiB).
Each ProDOS volume is 65535 blocks (33,553,920 B), so there's a
harmless 512-byte gap before the next partition. The firmware
math is in `Firmware/src/Helper.s::GETBLOCK`: SD address is
`(SMBASE × 65536 + BLOCKNUM)` blocks, scaled ×512 for byte-addressed
(SDSC) cards.

The boot ROM only autoboots partition 0; the other seven are
ProDOS-visible but not the autoboot target.

## Card choice

Any small SD works:
- **SDSC** (≤ 2 GB) — byte-addressed by the firmware.
- **SDHC** (4 GB – 32 GB) — block-addressed; only the first
  256 MiB is used.
- SDXC is untested but should appear as SDHC-like to the
  firmware.

Smaller is fine; the firmware detects card type via OCR and
adjusts addressing (`SDHC` flag in `$C0n3`).

## Recipe A — CiderPress Volume Copier (canonical, Windows)

The original AppleIISd tutorial
([Blue Meanie, 2017](https://bluemeanie-retro.blogspot.com/2017/12/how-to-prepare-sd-cards-for-appleiisd.html))
uses the Windows **CiderPress** GUI:

1. Run **CiderPress** as administrator (raw disk access needs it).
2. *File → Open Volume* → pick the SD card. Triple-check the
   device — picking the wrong one overwrites your system drive.
3. Use the **Volume Copier** to write a `.po` image into each
   partition slot. *Load from file* copies a ProDOS image into the
   selected slot.
4. For a bootable card, partition 0 must contain `PRODOS` and a
   `*.SYSTEM` file at the root.
5. Properly eject the card (taskbar icon) before pulling it.

Notes:
- A fresh SD card may show up as an **MS-DOS volume** the first
  time you open it. That's normal — CiderPress will overwrite it.
- CiderPress (Windows) has a **256 MB minimum** for this workflow.
  Smaller cards (e.g. 128 MB) won't hold 4+ ProDOS volumes.

On Linux/Mac, use Recipe B or C below — same end result, just dd.

## Recipe B — boot from a ready-made image (fast path, Linux/Mac)

Most curated 32MB ProDOS images (e.g. apple-2.com's BurgerDisk,
SmartPort, XDrive2C, etc., or Total Replay, Wizard Replay) are
already `.po` files exactly 33,553,920 bytes long — i.e. a raw
ProDOS volume ready to drop on disk.

1. Insert the SD card. Find its device node with `lsblk`. You
   want the disk (e.g. `/dev/sdc`), not a partition
   (`/dev/sdc1`). Unmount any auto-mounted volume.
2. Write the image to the start of the card:
   ```
   sudo dd if=burger32.po of=/dev/sdX bs=1M conv=fsync status=progress
   sync
   ```
3. Pop the card into the AppleIISd. Boot. Partition 0 is live.

## Recipe C — fill more partitions

Same `dd`, just `seek=N` where N is the partition number:

```
sudo dd if=games.po       of=/dev/sdX bs=32M seek=1 conv=fsync   # partition 1
sudo dd if=wizardry.po    of=/dev/sdX bs=32M seek=2 conv=fsync   # partition 2
sudo dd if=total.replay.po of=/dev/sdX bs=32M seek=3 conv=fsync  # partition 3
```

`bs=32M` is base-2 (32 MiB) in GNU dd, so it lines up with the
stride exactly. ProDOS 2.5 + firmware v1.3.0 will see all of
them as unit 1–N on the card's slot.

Reminder from `docs/4am-replay-collections-prodos-2.5.md`: Total
Replay / Instant Replay use a ProRWTS fast loader that breaks
on partitions 3–8 under ProDOS 2.5. Put those on partition 1
or 2.

## Recipe D — create a blank 32MB volume

Use **CiderPress II** (`cp2`, cross-platform .NET 8 CLI from
<https://github.com/fadden/CiderPress2>):

```
cp2 create-disk-image blank.po 32m ProDOS
```

That gives you a 33,554,432-byte `.po` with a blank ProDOS
filesystem and the default volume name. Then `dd` it onto the
card at whichever partition offset you want.

Set the volume name when you create it, or rename later in any
ProDOS file manager.

## Recipe E — format from inside the Apple II

If you have ProDOS booting from somewhere else (a floppy, another
SD card, a different mass-storage card), boot it, then format the
AppleIISd partitions in place using **FILER**, **Copy II Plus**,
**ProSEL-16**, or any ProDOS filer that can format volumes. The
firmware presents all 8 partitions as separate SmartPort devices
regardless of whether they hold a valid ProDOS volume, so the
format command Just Works once another system is up.

## Verifying

After booting:
- ProDOS 2.5: `CAT /` on each volume should show them as separate
  devices. `LIST DEVICES` (in BASIC.SYSTEM) or any catalog
  utility should enumerate 8 devices on the card's slot.
- ProDOS 2.4 and earlier: only 4 of the 8 are reachable per slot
  (the phantom-slot mapping in README §"Smartport drive remapping"
  applies).

## Caveat: reformatting after first use

If you `dd` a new image over a partition that ProDOS has open
(rare — ProDOS doesn't keep volumes mounted across reboots, but
it does cache directory blocks), reboot before using the new
content. The Apple II's ProDOS has no notion of "device removed"
the way modern OSes do.
