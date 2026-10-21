#include "game.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <knownfolders.h>
#include <shlobj.h>
#include <tlhelp32.h>

#include <cwchar>
#include <fstream>
#include <iterator>

static_assert(sizeof(void*) == 4, "the launcher must be 32-bit, like the game: it loads a 32-bit DLL into it");

namespace openbarrio::launcher {
namespace {

struct Build {
    const wchar_t* sha256;
    const wchar_t* name;
};
// SHA-256 of TS3W.exe of the official builds the redirect is verified on.
constexpr Build kSupportedBuilds[] = {
    {L"10b2423e2368e161774462b0ef1960231ad0af298d564a7bb6a6e78c5e880609", L"Steam 1.67.2"},
};

std::wstring RegistryString(HKEY root, const wchar_t* key, const wchar_t* value, DWORD flags)
{
    wchar_t buffer[1024];
    DWORD bytes = sizeof buffer;
    if (RegGetValueW(root, key, value, RRF_RT_REG_SZ | flags, nullptr, buffer, &bytes) != ERROR_SUCCESS)
        return L"";
    return buffer;
}

bool IsFile(const std::wstring& path)
{
    const DWORD attributes = GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

std::wstring FromUtf8(const std::string& text)
{
    const int length = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    std::wstring wide(static_cast<size_t>(length), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), wide.data(), length);
    return wide;
}

// The "path" entries of Steam's steamapps\libraryfolders.vdf (backslashes are escaped there).
std::vector<std::wstring> SteamLibraries(const std::wstring& steam)
{
    std::vector<std::wstring> libraries{steam};
    std::ifstream file(steam + L"\\steamapps\\libraryfolders.vdf", std::ios::binary);
    const std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    for (size_t at = text.find("\"path\""); at != std::string::npos; at = text.find("\"path\"", at + 6)) {
        const size_t open = text.find('"', at + 6);
        if (open == std::string::npos)
            break;
        std::string path;
        size_t i = open + 1;
        for (; i < text.size() && text[i] != '"'; ++i) {
            if (text[i] == '\\' && i + 1 < text.size())
                ++i;
            path += text[i];
        }
        libraries.push_back(FromUtf8(path));
        at = i;
    }
    return libraries;
}

}  // namespace

std::vector<std::wstring> FindGameCandidates()
{
    std::vector<std::wstring> found;
    auto add = [&](const std::wstring& exe) {
        if (!exe.empty() && IsFile(exe)) {
            for (const std::wstring& known : found)
                if (_wcsicmp(known.c_str(), exe.c_str()) == 0)
                    return;
            found.push_back(exe);
        }
    };
    std::wstring steam = RegistryString(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", L"SteamPath", 0);
    if (steam.empty())
        steam = RegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Valve\\Steam", L"InstallPath", RRF_SUBKEY_WOW6432KEY);
    for (wchar_t& c : steam)
        if (c == L'/')
            c = L'\\';
    if (!steam.empty())
        for (const std::wstring& library : SteamLibraries(steam))
            add(library + L"\\steamapps\\common\\The Sims 3\\Game\\Bin\\TS3W.exe");
    const std::wstring installDir =
        RegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Sims\\The Sims 3", L"Install Dir", RRF_SUBKEY_WOW6432KEY);
    if (!installDir.empty())
        add(installDir + (installDir.back() == L'\\' ? L"" : L"\\") + L"Game\\Bin\\TS3W.exe");
    return found;
}

const wchar_t* SupportedBuild(const std::wstring& sha256)
{
    for (const Build& build : kSupportedBuilds)
        if (sha256 == build.sha256)
            return build.name;
    return nullptr;
}

std::wstring InstallFolder(const std::wstring& exe)
{
    std::wstring folder = exe.substr(0, exe.find_last_of(L'\\'));  // ...\Game\Bin
    for (int up = 0; up < 2 && folder.find_last_of(L'\\') != std::wstring::npos; ++up)
        folder.resize(folder.find_last_of(L'\\'));
    return folder;
}

std::vector<std::wstring> RunningGamePrograms()
{
    std::vector<std::wstring> running;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return running;
    PROCESSENTRY32W entry{sizeof entry};
    for (BOOL more = Process32FirstW(snapshot, &entry); more; more = Process32NextW(snapshot, &entry))
        for (const wchar_t* name : {L"TS3W.exe", L"TS3.exe", L"Sims3Launcher.exe"})
            if (_wcsicmp(entry.szExeFile, name) == 0)
                running.emplace_back(name);
    CloseHandle(snapshot);
    return running;
}

std::wstring DocumentsFolder()
{
    wchar_t* path = nullptr;
    std::wstring result;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &path)))
        result = path;
    CoTaskMemFree(path);
    return result;
}

std::wstring StartGame(const std::wstring& exe, const std::wstring& dll, const std::wstring& arguments)
{
    std::wstring command = L"\"" + exe + L"\"";
    if (!arguments.empty())
        command += L" " + arguments;
    const std::wstring folder = exe.substr(0, exe.find_last_of(L'\\'));
    STARTUPINFOW startup{sizeof startup};
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(exe.c_str(), command.data(), nullptr, nullptr, FALSE, dll.empty() ? 0 : CREATE_SUSPENDED,
                        nullptr, folder.c_str(), &startup, &process))
        return L"Windows couldn't start the game (error " + std::to_wstring(GetLastError()) + L").";
    if (!dll.empty()) {
        // The game's main thread is still suspended: a thread of ours in the game runs LoadLibraryW(dll), and the
        // DLL's start-up code patches the game before the game's own code ever runs.
        const size_t bytes = (dll.size() + 1) * sizeof(wchar_t);
        void* remote = VirtualAllocEx(process.hProcess, nullptr, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        DWORD loaded = 0;
        if (remote && WriteProcessMemory(process.hProcess, remote, dll.c_str(), bytes, nullptr)) {
            auto* loadLibrary = reinterpret_cast<LPTHREAD_START_ROUTINE>(
                reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW")));
            HANDLE thread = CreateRemoteThread(process.hProcess, nullptr, 0, loadLibrary, remote, 0, nullptr);
            if (thread) {
                WaitForSingleObject(thread, 60000);
                GetExitCodeThread(thread, &loaded);
                CloseHandle(thread);
            }
        }
        if (remote)
            VirtualFreeEx(process.hProcess, remote, 0, MEM_RELEASE);
        if (loaded == 0) {
            TerminateProcess(process.hProcess, 1);
            CloseHandle(process.hThread);
            CloseHandle(process.hProcess);
            return L"The redirect couldn't be set up, so the game was stopped before it started.";
        }
        ResumeThread(process.hThread);
    }
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return L"";
}

}  // namespace openbarrio::launcher
