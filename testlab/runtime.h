#pragma once
#include "logic.h"
#include <filesystem>
#include <functional>
namespace openbarrio::lab {
namespace fs = std::filesystem;
std::string ReadText(const fs::path& path);
void WriteText(const fs::path& path, const std::string& text);
void WriteIni(const fs::path& path, const std::wstring& text);
std::wstring Wide(const std::string& text);
std::string Utf8(const std::wstring& text);
fs::path NewSession(const fs::path& root);
struct Run {
    fs::path exe, dll, session;
    std::vector<Hook> hooks;
    Selection enabled;
    bool replace = false;
};
struct RunResult { bool crashed = false, started = false; uint32_t exitCode = 0; std::string error; };
using Notify = std::function<void(const std::wstring&)>;
RunResult Play(const Run& run, const Notify& notify);
// Only used by the synthetic debugger tests; no injection and no game code.
RunResult DebugFixture(const fs::path& exe, const std::wstring& arg, const fs::path& session);
struct BuildResult { bool ok = false; fs::path directory; std::string error; };
BuildResult Build(const fs::path& source, const fs::path& runningExe, unsigned preset, const fs::path& log, const Notify& notify);
void CopyData(const std::wstring& source, const std::wstring& base, const std::wstring& gameFolder, const Notify& notify);
} // namespace openbarrio::lab
