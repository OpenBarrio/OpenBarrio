OpenBarrio save mover 0.1.0
===========================

Moves your Sims 3 saves, mods and settings to a folder you choose (for example out of OneDrive), removes the
read-only flags that make saving fail with "error 16", and starts the game so it uses the new folder.

Requires an original copy of The Sims 3: the Steam version 1.67.2 for now.


How to use it
-------------
1. Unzip both files (OpenBarrio.exe and openbarrio_redirect.dll) into one folder, for example
   D:\Games\OpenBarrio. Keep them together.
2. Start OpenBarrio.exe. It finds the game (or use "Find the game..." and pick TS3W.exe in
   The Sims 3\Game\Bin).
3. "Choose..." a folder for your data. You can pick the folder itself, its "Electronic Arts" folder or its
   "Electronic Arts\The Sims 3" folder: OpenBarrio works out which one you mean. Then either:
   - "Copy my data there": copies your data from Documents (Electronic Arts\The Sims 3) into it. That folder
     isn't changed and stays as a backup. Delete it yourself once you're happy.
   - "Use this folder (no copy)": uses the folder as it is. If you already copied your data there yourself,
     the game uses it (OpenBarrio removes its read-only flags). If the folder is empty, the game starts fresh
     there, without your saves from Documents.
4. Click "Launch The Sims 3". From now on, start the game this way.

Only want to fix error 16 without moving anything? Click "Remove read-only flags".


What it does, exactly
---------------------
- "Launch The Sims 3" starts the game's own TS3W.exe and loads openbarrio_redirect.dll into it before the game
  starts. That DLL changes one thing in the running game: the folder where the game keeps your user data. It
  replaces one function of the game in memory, and only if that function is exactly the one of the supported
  version; otherwise it changes nothing and the game isn't started.
- Nothing in the game folder is changed. No files are written outside the folder you chose and the folder
  OpenBarrio.exe is in (openbarrio.ini with your settings, openbarrio_hook.log with the last start's result).
- No internet access, no admin rights, nothing installed, nothing runs in the background.
- To undo it: click "Use the original Documents folder", or simply start the game the usual way again. Your
  data stays wherever you put it.


Known limits
------------
- Start the game through OpenBarrio. EA's own launcher (Sims3Launcher.exe, used for the store, downloads and
  .sims3pack installs) still uses the old folder.
- A few script-side folder lookups in the game aren't redirected yet.
- Only the Steam version 1.67.2 is supported. Other versions are refused rather than changed.
- It doesn't fix error 12 (the game running out of memory while saving).


Antivirus and Windows warnings
------------------------------
OpenBarrio isn't signed, so Windows SmartScreen may warn when you start it ("More info", then "Run anyway").
Programs that load a DLL into a game can also be flagged by antivirus software. You can check that your files
are the published ones: the SHA-256 of the zip is listed on the release page, and the source code and build
instructions are at the project's page. The build is reproducible, so anyone can rebuild the same files.


OpenBarrio is free software under the GPL-3.0 license (see LICENSE). It is not affiliated with or endorsed by
Electronic Arts Inc. or Maxis. The Sims is a trademark of Electronic Arts Inc.
