// Finding the game, checking that it is a supported original copy, and starting it with the redirect DLL.
#pragma once

#include <string>
#include <vector>

namespace openbarrio::launcher {

// TS3W.exe files found through Steam's library list and the game's registry keys (only ones that exist).
std::vector<std::wstring> FindGameCandidates();
// The supported official build with this SHA-256 ("Steam 1.67.2"), or nullptr for anything else.
const wchar_t* SupportedBuild(const std::wstring& sha256);
// The install folder ("...\The Sims 3") of a TS3W.exe in "...\The Sims 3\Game\Bin".
std::wstring InstallFolder(const std::wstring& exe);
// Names of running programs that must be closed before the data is copied or the game is started.
std::vector<std::wstring> RunningGamePrograms();
// The user's Documents folder (follows OneDrive's redirection, like the game does).
std::wstring DocumentsFolder();

// Starts the original TS3W.exe with `arguments`. With a DLL, the game starts suspended, the DLL is loaded into it,
// and only then does the game run; if the DLL refuses to load, the game is stopped before it ran any of its code.
// Returns "" on success, else what went wrong.
std::wstring StartGame(const std::wstring& exe, const std::wstring& dll, const std::wstring& arguments);

}  // namespace openbarrio::launcher
