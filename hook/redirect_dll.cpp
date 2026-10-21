// openbarrio_redirect.dll: loaded into TS3W.exe by OpenBarrio.exe while the game's main thread is still suspended,
// so before the game runs any of its own code. It makes the game keep its user data in the folder the player chose
// (openbarrio.ini next to this DLL, [redirect] documents_dir) by replacing one function of the game, TS3W.exe
// 0x004DC100 (the special-folder lookup), with our rewrite ts3::io::SpecialDirectory. Only the game's memory is
// changed; nothing on disk is.
//
// The patch is a 5-byte jump at the start of the original function. It is only written when the 8 bytes there are
// exactly the ones of the supported build (Steam 1.67.2). On anything else the DLL patches nothing and fails to
// load, so the launcher stops the game rather than letting it quietly use the old folder.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdint>
#include <cstring>
#include <cwchar>
#include <string>

#include "ts3/io/special_dir.h"

namespace {

constexpr uintptr_t kGameImageBase = 0x00400000;
constexpr uintptr_t kSpecialDirectory = 0x004DC100;
constexpr uint8_t kExpectedBytes[8] = {0x81, 0xEC, 0x0C, 0x02, 0x00, 0x00, 0x53, 0x55};
// SpecialDirectory appends a separator to the base and fails at 260 characters.
constexpr size_t kMaxBaseLength = 258;

std::wstring g_folder;  // this DLL's folder, with a trailing separator

int __cdecl RedirectedSpecialDirectory(int folderId, wchar_t* path, char requireDirectory, uint32_t capacity)
{
    return ts3::io::SpecialDirectory(folderId, path, requireDirectory, capacity);
}

// One line per load, replacing the previous log, in UTF-8.
void Log(const wchar_t* message)
{
    const std::wstring path = g_folder + L"openbarrio_hook.log";
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return;
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, message, -1, nullptr, 0, nullptr, nullptr);
    if (bytes > 1) {
        std::string text(static_cast<size_t>(bytes - 1), '\0');
        WideCharToMultiByte(CP_UTF8, 0, message, -1, text.data(), bytes, nullptr, nullptr);
        text += "\r\n";
        DWORD written = 0;
        WriteFile(file, text.data(), static_cast<DWORD>(text.size()), &written, nullptr);
    }
    CloseHandle(file);
}

bool IsSupportedGameProcess()
{
    wchar_t exe[MAX_PATH];
    const DWORD length = GetModuleFileNameW(nullptr, exe, MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
        return false;
    const wchar_t* name = std::wcsrchr(exe, L'\\');
    name = name ? name + 1 : exe;
    return _wcsicmp(name, L"TS3W.exe") == 0 &&
           reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr)) == kGameImageBase;
}

bool WriteJump(uintptr_t at, const void* target)
{
    auto* code = reinterpret_cast<uint8_t*>(at);
    DWORD oldProtect = 0;
    if (!VirtualProtect(code, sizeof kExpectedBytes, PAGE_EXECUTE_READWRITE, &oldProtect))
        return false;
    const bool expected = std::memcmp(code, kExpectedBytes, sizeof kExpectedBytes) == 0;
    if (expected) {
        const int32_t displacement = static_cast<int32_t>(reinterpret_cast<uintptr_t>(target) - (at + 5));
        code[0] = 0xE9;  // jmp rel32
        std::memcpy(code + 1, &displacement, sizeof displacement);
    }
    VirtualProtect(code, sizeof kExpectedBytes, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), code, sizeof kExpectedBytes);
    return expected;
}

bool Install(HMODULE self)
{
    wchar_t module[MAX_PATH];
    const DWORD length = GetModuleFileNameW(self, module, MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
        return false;
    g_folder.assign(module, length);
    g_folder.resize(g_folder.find_last_of(L'\\') + 1);

    if (!IsSupportedGameProcess()) {
        Log(L"FAILED: not loaded into the supported TS3W.exe; nothing was changed.");
        return false;
    }
    wchar_t base[1024] = {};
    const std::wstring ini = g_folder + L"openbarrio.ini";
    GetPrivateProfileStringW(L"redirect", L"documents_dir", L"", base, 1024, ini.c_str());
    const size_t baseLength = std::wcslen(base);
    if (baseLength == 0 || baseLength > kMaxBaseLength) {
        Log(L"FAILED: no usable documents_dir in openbarrio.ini; nothing was changed.");
        return false;
    }
    ts3::io::SetUserDocumentsDir(base);
    if (!WriteJump(kSpecialDirectory, reinterpret_cast<const void*>(&RedirectedSpecialDirectory))) {
        Log(L"FAILED: the game's code at 0x004DC100 is not the expected Steam 1.67.2 code; nothing was changed.");
        return false;
    }
    Log((std::wstring(L"OK: the game's Documents folder is now ") + base).c_str());
    return true;
}

}  // namespace

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    if (reason != DLL_PROCESS_ATTACH)
        return TRUE;
    DisableThreadLibraryCalls(instance);
    return Install(instance) ? TRUE : FALSE;
}
