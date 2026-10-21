# OpenBarrio

OpenBarrio brings tools and engines rewrites targeting The Sims 3. These are sets of functions written from scratch against the original game.

This is a reverse-engineering attempt to fix many bugs that plague The Sims 3 for almost the last 20 years. 

The project focus on keeping as much as possible from the original and generate patches for common bugs such as errors 16 and 12.

### Disclaimer

This project contains LLM assisted coding. A loop with Claude coordinating workers on Sol and Qwen3.8-Flash has been used to reverse-engineer the code. The code has been tested with an automated gameplay loop (as well as with my wife playing the game) :)

### Status

**Super early!** This repository contains only a few of the functions already reworked. I will be adding them as soon as they are concluded.

### Who am I and why OpenBarrio exists

I am a chip designer at a semiconductor company in Europe. My objective with this project is to ride the reverse engineering waves and finally fix a game that my wife and I love. 

While software coding is not my trade, this is also an attempt to improve my skills in managing LLMs which are being steadily more used in my industry. 

But don't worry! I will only commit code that has been fully verified by my testing loop as well as gameplay between me and her. 

## LLM Stuff

Below is some machine written README stuff thanks to Claude. It is some context about the first release which fixes Error 16.

You should read about the save mover and the rest is just some LLM stuff about build, progress and legal.

## Save mover: the fix for error 16 (release 0.1.0)

When Windows keeps Documents in OneDrive, The Sims 3 keeps its saves there too, and OneDrive's syncing and the
read-only flags it leaves on files make saving fail ("error 16"). The game has no setting to move its folder.

`OpenBarrio.exe` is one small window that:

1. finds the game and checks that it is an original copy (the Steam version 1.67.2 for now);
2. lets you choose a new folder for your saves, mods and settings, for example outside OneDrive (the folder
   itself, its `Electronic Arts` folder or its `The Sims 3` folder: it works out which one you mean);
3. either copies your data there, or uses the folder as it is (data you copied there yourself, or a fresh start in
   an empty folder). A copy is checked, saves are compared byte for byte, your old folder isn't changed and stays
   as a backup, and existing data is never mixed with another set;
4. removes the read-only flags (also on its own, with "Remove read-only flags", if you only want that);
5. starts the game with "Launch The Sims 3".

**What it does, exactly.** "Launch The Sims 3" starts the game's own `TS3W.exe` and loads
`openbarrio_redirect.dll` into it before the game starts. The DLL replaces one function of the game in memory, the
lookup of the user-data folder, with our rewrite (TS3W.exe `0x004DC100`, in
[src/core/io/special_dir.cpp](src/core/io/special_dir.cpp)), and only after checking that the function is exactly
the one of the supported version. Otherwise it changes nothing and the game isn't started. The source is in
[hook/](hook/), [launcher/](launcher/) and [mover/](mover/).

**What it never does:** change anything in the game folder, write outside the folder you chose and its own folder,
use the internet, ask for admin rights, install anything or run in the background.

**Known limits:** start the game through OpenBarrio (EA's Sims3Launcher still uses the old folder); a few
script-side folder lookups aren't redirected yet; only Steam 1.67.2 is supported; it doesn't fix error 12.

**Antivirus.** The program isn't signed, so SmartScreen may warn, and tools that load a DLL into a game are
sometimes flagged by antivirus software. Check the download against the SHA-256 on the release page, or build it
yourself: the build is reproducible.

## What's included

320 rewritten functions in a static library (`openbarrio_core`). The full list, with each function's address in
the original game, is in [docs/coverage.md](docs/coverage.md).

| Part | Functions |
|---|---|
| Package reader: DBPF package reader, RefPack decompression, FNV hashes, packed records | 41 |
| Streams and file I/O | 69 |
| Core runtime: threads, thread-local storage, job manager, stopwatch, allocator helpers | 136 |
| Folder redirect: the user-data folder lookup the save mover replaces | 1 |
| Other verified functions these depend on | 73 |

The code runs inside the game's process: it uses the original's memory layouts, and some functions still call
original code that isn't rewritten yet, so we still depend on the original The Sims 3 code.

## Building

Requirements: Windows, Visual Studio 2026 (MSVC toolset 14.5x) and CMake 3.28 or newer.

```
cmake --preset x86
cmake --build --preset x86-release
cmake --preset x64
cmake --build --preset x64-release
```

The libraries end up in `build86/Release/` and `build64/Release/`. The x86 build also makes the save mover
(`build86/bin/Release/`) and its tests (`ctest --test-dir build86 -C Release`). To make the release zip:

```
python tools/package.py
```

Builds are reproducible: building the same commit twice gives byte-identical files and the same zip.

| Release | File | SHA-256 |
|---|---|---|
| 0.1.0 | `OpenBarrio-0.1.0.zip` | `471dbd84e31bfc23c8b5724759d02f73962423878105814f1de47fd2e83d0813` |

## Legal (attempt)

- **Our own code.** Everything in `src/` was written for this project. It contains no code, data or assets from
  the game: no decompiled or disassembled code and no game files. Comments cite addresses in the original
  executable so that each rewrite can be checked against the function it replaces.
- **Original copies only.** Using this code requires a legally owned, installed copy of The Sims 3. Nothing here
  helps run unlicensed copies, and nothing touches copy protection.
- **Interoperability.** The work is done for interoperability with the original program, within the limits of
  EU Directive 2009/24/EC on the legal protection of computer programs (Article 6, decompilation for
  interoperability).
- **Not affiliated** with or endorsed by Electronic Arts Inc. or Maxis. The Sims is a trademark of Electronic Arts
  Inc.
- **Third-party code:** EA's open-source EASTL and EABase libraries are included unmodified under their BSD
  3-Clause license. See [THIRD_PARTY.md](THIRD_PARTY.md).

## License

GPL-3.0. See [LICENSE](LICENSE).
