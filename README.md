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

## Features

### Save mover: the fix for error 16 (release 0.1.0)

The save mover is the first release from OpenBarrio. It essentially is a tool that changes the engine code to move your `Electronic Arts` folder to a different place.

By default EA has hardcoded the `Electronic Arts` folder to live inside `Documents`, which can generate conflicts with OneDrive.
This conflict appears in-game as **Error 16** when saving. The simple fix at the moment is to move the folder to outside of OneDrive.

What the release of `OpenBarrio.exe` does:

1. Finds the game and checks that it is an original copy (the Steam version 1.67.2 for now).
2. Lets you choose a new folder for your saves, mods and settings.
3. Gives you the option to copy your data there, or uses the folder as it is (you can copy it yourself if you want).
4. Removes the read-only flags (with the "Remove read-only flags" button).
5. Starts the game with "Launch The Sims 3" and inserts the new code to the game.

#### Technical Explanation

It launches The Sims 3" own `TS3W.exe` and loads `openbarrio_redirect.dll` into it before the game starts. 
The DLL replaces a function of the game responsible to looking-up the user-data folder with the rewritten code (TS3W.exe `0x004DC100`, in
[src/core/io/special_dir.cpp](src/core/io/special_dir.cpp)). Unfortunately it can only work with the latest Steam version for now.
The source is in [hook/](hook/), [launcher/](launcher/) and [mover/](mover/).

**Known limits:** start the game through OpenBarrio (EA's Sims3Launcher still uses the old folder); a few
script-side folder lookups aren't redirected yet; only Steam 1.67.2 is supported; it doesn't fix error 12.

**Antivirus.** The program isn't signed, so SmartScreen may warn, and tools that load a DLL into a game are
sometimes flagged by antivirus software. Check the download against the SHA-256 on the release page, or build it
yourself: the build is reproducible.

## The current status of the project

Many functions have been rewritten but currently only 320 rewritten functions are available as a static library (`openbarrio_core`). 
The full list, with each function's address in the original game, is in [docs/coverage.md](docs/coverage.md) (the document has been written by an LLM).

| Part | Functions |
|---|---|
| Package reader: DBPF package reader, RefPack decompression, FNV hashes, packed records | 41 |
| Streams and file I/O | 69 |
| Core runtime: threads, thread-local storage, job manager, stopwatch, allocator helpers | 136 |
| Folder redirect: the user-data folder lookup the save mover replaces | 1 |
| Other verified functions these depend on | 73 |

The code runs inside the game's process: it uses the original's memory layouts, and some functions still call
original code that isn't rewritten yet, so it still depend on the original The Sims 3 code.

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

Information in case you are looking to reproduce the build:

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
