# OpenBarrio

OpenBarrio brings tools and engines rewrites targeting The Sims 3. These are sets of functions written from scratch against the original game as a reverse-engineering attempt to fix many bugs that plague The Sims 3 for almost 20 years. 

OpenBarrio acts as a DLL injector, swapping the original engine code for the rewritten version. Currently it attempts to do a 1:1 translation with patches being the future goal.

### Disclaimer

This project contains LLM assisted coding. A loop with Claude coordinating workers on Sol and Qwen3.8-Flash has been used to reverse-engineer the code. The code has been tested with an automated gameplay loop (as well as with my wife playing the game) :)

### Status

**Super early!** This repository contains only a few of the functions already reworked. I will be adding them as soon as they are concluded.

### Who am I and why OpenBarrio exists

I am a chip designer at a semiconductor company in Europe. My day job is in ASIC design and verification using HDL, TCL and occasional python scripts. 

The objective of this project is to create a repository of code that initially can replace the original engine and eventually create fixes for a game that my wife and I love. 

## Features

### Test Lab and Save mover: 

As of October 23rd users can now toggle rewritten functions and debug failing functions with the OpenBarrio Test Lab. The Test Lab is a tool that integrates the save game mover and debugging tools to log where miss-matches happen in the code. 

See [the Test Lab guide](testlab/README.md) for use, automated checks, and the game tests still required.

<img width="742" height="609" alt="image" src="https://github.com/user-attachments/assets/8ce50a31-5473-49b6-8b1c-cdcaff9d971f" />

#### Old Save mover (fix for Error 16)

The save mover was the first release from OpenBarrio. It essentially is a tool that changes the engine code to move your `Electronic Arts` folder to a different place.

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

<img width="1177" height="1030" alt="image" src="https://github.com/user-attachments/assets/80cc3d0f-1605-4f38-9e84-ac14854b5f3d" />

The code runs inside the game's process: it uses the original's memory layouts, and some functions still call original code that isn't rewritten yet, so it still depend on the original The Sims 3 code.

## Building

Requirements: Windows, Visual Studio 2026 (MSVC toolset 14.5x) and CMake 3.28 or newer.

```
cmake --preset x86
cmake --build --preset x86-release
cmake --preset x64
cmake --build --preset x64-release
```

The libraries end up in `build86/Release/` and `build64/Release/`. The x86 build also makes the save mover (`build86/bin/Release/`) and its tests (`ctest --test-dir build86 -C Release`). 

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
