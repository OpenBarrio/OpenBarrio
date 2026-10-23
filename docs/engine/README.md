# Engine rewrites

Functions of The Sims 3 engine (`TS3W.exe`, Steam 1.67.2) rewritten in portable C++. Each one carries the address
of the original function it replaces and was checked against it: the original and the rewrite run side by side on
the same inputs and must give identical results. Only files whose functions have **all** passed that check are here.

**5,385 functions** in 647 source files (plus 676 headers), under [`src/core/`](../../src/core/).

## By module

| Module | Functions | Main folders |
|---|---:|---|
| Package reader (pilot) | 124 | `resource/` |
| m01 Streams and file I/O | 104 | `io/` |
| m02 Core runtime | 140 | `runtime/`, `thread/` |
| m03 Resources | 672 | `resource/`, `resourcesystem/`, `resourceload/` |
| m04 Math | 218 | `math/` |
| m05 Animation | 1,368 | `anim/`, `animation/` |
| m06 Audio | 709 | `audio/` |
| m07 Renderer | 672 | `render/`, `renderer/`, `lotrenderer/`, `lighting/`, `shader/` |
| Shared support code (outside the module lists) | 1,378 | `misc/`, `scratch/`, `world/`, `ui/`, `cas/`, ... |

Files named `mNN_...` belong to a module batch; files named `clu_ADDRESS_Subsystem` / `cls_ADDRESS_...` group
functions found together near that address. Headers are in `src/core/include/ts3/`, by subsystem.

## The lists

| File | What |
|---|---|
| [FUNCTIONS.csv](FUNCTIONS.csv) | one row per function: module, original address, name, file, how it was verified, its hook |
| [HOOKS.csv](HOOKS.csv) | the 1,499 hooks in the test harness, with the in-game test they passed and whether compare mode can run them |
| [DROPPED_HOOKS.csv](DROPPED_HOOKS.csv) | 251 hooks left out of the harness because their code needs a function that isn't released yet |
| [FILES.csv](FILES.csv) | every released file: kind, path, function count, modules |
| [LEFT_OUT.csv](LEFT_OUT.csv) | 59 files not released yet: 49 still have unverified functions, 10 have a hook that was switched off in game |

"verified offline" in FUNCTIONS.csv means the side-by-side check against the original; functions marked with an
in-game hook state also ran inside the game. Functions without a hook are called by other released code.

## Test harness

[`harness/`](../../harness/) builds `openbarrio_test.dll` and `openbarrio_launch.exe` (32-bit, like the game). The
launcher starts an original, unmodified copy of the game (it checks the SHA-256 of `TS3W.exe`) with the DLL, which
installs the hooks. **Compare mode** is the default: both versions run, the game keeps the original result, and
differences are logged to `openbarrio_test.log`. **Replace mode** runs the rewrites instead of the originals; back up
your saves first. Details in [harness/README.txt](../../harness/README.txt) and `harness_public.ini`.

Only hooks that were ON in a passing in-game test (world load, play, save and reload) are in the harness; functions
verified since then are in `src/core/` and get their hooks after their in-game test.

## Build

```
cmake --preset x86 && cmake --build --preset x86-release   # core, test harness, save mover
cmake --preset x64 && cmake --build --preset x64-release   # the 64-bit core
```

The core is a static library: some functions call code that isn't released yet, so linking everything into one
program leaves unresolved symbols. The harness links only what its hooks use.
