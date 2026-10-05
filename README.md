# Dodge the Falling Obstacles

An 8086-compatible, 16-bit real-mode DOS game built as a single `.COM` file.
The four source files are combined by NASM `%include`; no linker or libraries
are needed.

| File | Responsibility |
| --- | --- |
| `main.asm` | COM entry, stack/BIOS setup, state machine, tick-paced loop, data |
| `render.inc` | Direct B800h VRAM drawing, row offsets, HUD, actor erasure |
| `physics.inc` | LCG randomness, falling, swept AABB collision, lives, levels |
| `input.inc` | Non-blocking BIOS keys, title, game over, restart |

## Build

Install NASM and DOSBox (or DOSBox Staging). Open a terminal in this project:

```sh
nasm -f bin main.asm -o dodge.com
```

Optional strict build with NASM 3.02:

```sh
nasm -f bin -Wall -w-reloc-abs-word -Werror main.asm -o dodge.com
```

NASM 3.02's optional `reloc-abs-word` warning flags the intentional absolute
16-bit addresses in this flat COM image, so that warning is excluded from the
strict build. `cpu 8086` rejects instructions requiring later CPUs.

## Run on Linux

Build and launch automatically (requires NASM and DOSBox):

```sh
./play.sh
```

The launcher also works when called by its full path from another directory.
DOSBox closes when you exit the game.

From the project directory:

```sh
dosbox -c 'mount c .' -c 'c:' -c 'dodge.com'
```

Or enter these commands at the DOSBox prompt:

```text
mount c /home/hien/Documents/DodgeObstacles
c:
dodge.com
```

## Run on Windows

Build with NASM from the project directory using the same build command.
For a project stored at `C:\Games\DodgeObstacles`, enter in DOSBox:

```text
mount c "C:\Games\DodgeObstacles"
c:
dodge.com
```

The built `dodge.com` is also portable between these hosts. Windows instructions
have not been executed on a Windows host.

## Play

- SPACE or ENTER starts from the title screen.
- A / Left Arrow moves left; D / Right Arrow moves right.
- ESC exits from any screen and restores standard DOS text mode.
- R restarts after game over.

The green three-cell player stays on row 23, within columns 0–77. Three
individually colored two-cell blocks fall through rows 1–24; row 0 is the HUD.
Holding a movement key uses the BIOS keyboard repeat behavior.

Start with three hearts. An overlapping block costs one life and respawns.
The player renders red for two gameplay frames after a hit, without pausing.
Multiple blocks hitting in one frame cost one life total; later frames can
cause another hit. Losing the last life immediately displays game over.

Each block dodged past the bottom earns one point. Level is `1 + score / 50`
(integer division); falling speed starts at one row per tick and increases by
one per level, capped at 23 rows per tick, the useful playfield traversal limit.
Score saturates at 65535. Collision checks the entire vertical distance crossed
by a block, including at speeds that skip row 23 visually.

BIOS INT 1Ah ticks pace updates at approximately 18.2 frames per second. The
full tick count is compared, including rollover. Slow host frames are not
followed by bursts of catch-up updates. INT 16h input is polled without waiting,
with a maximum of 16 queued keys processed per frame.

Rendering erases only the saved previous actor cells before redrawing actors
and the HUD. Full-screen clears occur only on screen transitions. VRAM offsets
use `rowOff` and shifts; the LCG also uses shift/add arithmetic. No custom
interrupt handlers are installed, so no interrupt-vector restoration is needed.

