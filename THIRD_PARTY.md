# Third-party code

OpenBarrio vendors two of Electronic Arts' open-source libraries, unmodified, in `third_party/`. Only their
headers, sources and license files are included.

| Library | Upstream | Commit | License |
|---|---|---|---|
| EASTL | https://github.com/electronicarts/EASTL | `e323f67e5d93fdadcd44f7998069d804697b1945` | BSD 3-Clause (`third_party/EASTL/LICENSE`, `3RDPARTYLICENSES.TXT`) |
| EABase | https://github.com/electronicarts/EABase | `0699a15efdfd20b6cecf02153bfa5663decb653c` | BSD 3-Clause (`third_party/EABase/LICENSE`) |

**Don't upgrade these commits.** The rewritten functions share container objects with the original game, so
their memory layout must match the EASTL version the code was verified with.
