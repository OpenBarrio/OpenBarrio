# OpenBarrio Test Lab

`OpenBarrioTestLab.exe` is a local Win32 test interface for the engine harness. It is an experimental tool for the original Steam TS3W.exe 1.67.2. The game executable must pass the existing SHA-256 check. Nothing is written to the game installation.

## Use it in five steps

1. Open `build86/Release/OpenBarrioTestLab.exe`. Keep `openbarrio_test.dll` and `HOOKS.csv` next to it. Keep the matching local `openbarrio_test.map` for crash attribution.
2. Select modules with All/None or individual hooks in Advanced. Compare is the default; only certified compare-safe hooks run. Save selection / Load selection stores reusable INI presets.
3. Optionally choose a user-data source and destination base, then Copy + verify. The existing mover copies and verifies the data, preserves the source, and clears read-only attributes on the copy. Use existing folder selects data already there. Normal Documents resets the choice.
4. Press Play. Replace mode warns and offers a verified backup of the active user-data tree before each run. Folder redirection works only in Replace and pins `io.SpecialDirectory` on. Compare uses normal Documents. Only the test DLL is injected; never combine it with the separate redirect DLL.
5. After exiting, select a session and open its folder, summary, or crash report. Select a crash session and choose Find the culprit to repeat the same scenario with smaller hook sets.

## Preset and Build button

Choose the OpenBarrio source folder, select `x86-release` or `x64-release`, and click Build. CMake, Visual Studio, and CTest must be installed and available as for the normal build. The window remains responsive and displays the live output. The x86 build also runs CTest; x64 builds the core library.

Builds configure the selected architecture preset into `out/testlab-x86` or `out/testlab-x64`, then build its Release configuration with one build job. A separate tree avoids overwriting the running GUI. If running from that tree, `-next` is used instead. Output opens the build folder. After a successful x86 build, Play uses that build's DLL and matching hook catalog; the current GUI keeps running. Open the new executable from the output folder to use updated GUI code. Builds are disabled during a culprit search to keep the subject stable.

The Build button does not modify the CMake presets or the released save-mover targets. Its log is `build.log` next to the running executable.

## Sessions and crash detection

Each run has a unique `sessions/YYYYMMDD_HHMMSS/` folder next to the executable (a numeric suffix prevents same-second collisions):

- `harness_public.ini`: exact UTF-16 INI used, with all hooks explicitly enabled or disabled.
- `runtime/`: private copy of the test DLL, its hook catalog, INI, local MAP if present, and the live harness log. This keeps another run or build from changing the running subject.
- `openbarrio_test.log`: log copied from the runtime folder when the process exits.
- `summary.txt`: mode, selected modules/hooks, elapsed time, exit code, crash classification, and the largest cumulative mismatch counts. Missing counters do not prove execution or success.
- `crash_report.txt` and, on an unhandled exception, `crash.dmp`. A dump-writing failure is reported explicitly.
- `build.txt`: DLL hash and supported game version; `backup/` if the player accepted a backup.

The debugger attaches after suspended-start DLL injection and before resuming the main thread. It passes first-chance exceptions through to the game, consumes only the debugger's initial breakpoint, and captures unhandled second-chance exceptions. A nonzero exit code alone is not called a crash. The GUI stays open until the game exits; it does not offer a force-kill button. An unexpected debugger observation failure is reported rather than inventing a crash result.

Crash reports name the module and offset. For the test DLL, the local linker MAP identifies the nearest symbol and matching/nearby rewritten hooks. Optimized or folded code and shared helpers make attribution approximate. For TS3W.exe, enabled original hook addresses below the exception are listed as proximity suspects. A suspect is not proof of causation.

The debugger approach passes synthetic handled-exception, unhandled-exception, module-attribution, and ordinary-exit tests. It has **not been validated in the game**. No exception watcher has been substituted: game testing is needed before deciding whether the debugger changes game behavior. Attach failure stops the suspended game and reports the error.

References: [Windows debugger attachment](https://learn.microsoft.com/en-us/windows/win32/debug/debugging-a-running-process), [minidump exception information](https://learn.microsoft.com/en-us/windows/win32/api/minidumpapiset/ns-minidumpapiset-minidump_exception_information).

## Find the culprit

Start from a completed session. The search snapshots that session's enabled set and keeps using its original DLL/catalog. It splits modules first, then individual hooks. The redirect hook stays on when using redirected saves and is excluded from that search. Repeat the same save and actions for every trial. A captured crash records the result automatically; otherwise select It crashed or No crash after the game exits.

The search saves `bisect.state` in the original session, with before/after snapshots in trial sessions. `testlab.ini` remembers the active search. An unfinished trial is restored for manual classification after restarting the GUI. A remaining single hook is tested again before it is marked reproduced. End search leaves the evidence on disk.

This assumes one reproducible faulty hook. Timing changes, interactions between hooks, or crashes in unmodified game code may leave no single culprit or identify a candidate that needs further confirmation. Keep the initial full-set reproduction as a control, and also try None (with only the redirect pinned if needed) before treating the candidate as the cause. Address proximity is not used to silently exclude other enabled hooks.

## Build and automated checks

```text
cmake --preset x86
cmake --build --preset x86-release --parallel 1
ctest --test-dir build86 -C Release --output-on-failure
```

CTest includes the existing mover suite, pure Test Lab logic (INI, CSV, module/hook bisection, persistence, MAP parsing, hook proximity, log counts, and quoting), a synthetic debugger fixture, and a hidden-window GUI smoke test. None launches the game. New targets use C++20, the static CRT, `/Brepro`, and `/W4 /WX`.

For the optional end-to-end Build-button test, run `testlab_gui_tests.exe` with the absolute Test Lab executable path and the absolute source-folder path. It opens a hidden fixture copy of the GUI, presses Build, waits for the full configure/build/CTest sequence, and closes it. Omit the source argument for the ordinary fast GUI test.

## Manual game checks still required

1. Compare, All: start the original game, load a test world, repeat representative gameplay, then exit normally. Check the session summary, installed-hook count, and log. Confirm unsafe comparisons were skipped.
2. Replace, one module: accept the backup, load a disposable test save, play, save and reload, then exit. Confirm the backup exists and the original source folder was retained.
3. Redirect: copy a test user-data tree to a new base, verify it, then use Replace. Confirm the game writes there. Return to Compare and confirm the UI says normal Documents is used.
4. On a reproducible crash: inspect the report/dump, start Find the culprit, repeat the identical trigger for each trial, close/reopen the GUI between trials, and confirm the search resumes. Retest the final candidate and the original full set. There is no deliberate-crash switch in the game-facing GUI; synthetic crashes are confined to the debugger fixture.

Session folders and dumps are ignored by Git and must never be published: they can contain game memory, saves, and local paths. MAP/PDB files are local diagnostics and are not release artifacts. The engine source and generated harness source are unchanged by this tool.
