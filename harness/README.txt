OpenBarrio test build 2 — Steam TS3W.exe 1.67.2 only

Keep openbarrio_test.dll and harness_public.ini together. Start the game with:
  openbarrio_launch.exe "<game folder>/TS3W.exe"
The launcher injects the DLL before the game's main thread starts. No game file
is modified. Do not unload the DLL while the game is running.

Compare is the default: both versions run, the game uses the original result,
and differences are logged. Hooks without a certified safe comparison are
skipped, NOT silently replaced. HOOKS.csv describes the available hooks.
"no (side effects)" also includes comparisons conservatively disabled because
their external effects could not be certified by the generator's leaf checks.

Replace runs the rewrites. A wrong rewrite can crash the game or corrupt a save.
BACK UP SAVES FIRST. Set mode=replace only if you accept this risk.
An empty user_documents_dir keeps the normal user documents location; an
explicit directory redirects that hook in replace mode and must already exist.

Send openbarrio_test.log (next to the DLL) with bug reports. It contains the build
version, mode, first five mismatch details per hook, and nonzero call/mismatch
counts every 60 seconds and at normal process exit. Forced termination or a
crash may prevent the final report. No background/window or diagnostic options
are included. DROPPED.csv lists selected hooks omitted for link dependencies.
