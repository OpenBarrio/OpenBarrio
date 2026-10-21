# OpenBarrio

OpenBarrio brings tools and engines rewrites targeting The Sims 3. These are sets of functions written from scratch against the original game.

This is a reverse-engineering attempt to fix many bugs that plague The Sims 3 for almost the last 20 years. 

The project focus on keeping as much as possible from the original and generate patches for common bugs such as errors 16 and 12.

**Disclaimer:** this project contains LLM assisted coding. A loop with Claude coordinating workers on Sol and Qwen3.8-Flash has been used to reverse-engineer the code. The code has been tested with an automated gameplay loop (as well as with my wife playing the game) :)

**Status: super early!** This repository contains only a few of the functions already reworked. I will be adding them as soon as they are concluded.

**Who am I:** I am a chip designer at a semiconductor company in Europe. My objective with this project is to ride the reverse engineering waves and finally fix a game that my wife and I love. While software coding is not my trade, this is also an attempt to improve my skills in managing LLMs which are being steadily more used in my industry. But don't worry! I will only commit code that has been fully verified by my testing loop as well as gameplay between me and her. 

## What's included

319 rewritten functions in a static library (`openbarrio_core`). The full list, with each function's address in
the original game, is in [docs/coverage.md](docs/coverage.md).

| Part | Functions |
|---|---|
| Package reader: DBPF package reader, RefPack decompression, FNV hashes, packed records | 41 |
| Streams and file I/O | 69 |
| Core runtime: threads, thread-local storage, job manager, stopwatch, allocator helpers | 136 |
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

The libraries end up in `build86/Release/` and `build64/Release/`. Builds are reproducible: building the same
commit twice gives byte-identical libraries. The SHA-256 of each release archive will be listed here from the
first release on.

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
