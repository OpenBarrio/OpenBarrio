#include "ts3/io/special_dir.h"
#include "ts3/layout.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shlobj.h>
#if defined(_MSC_VER)
#pragma comment(lib, "shell32.lib")
#endif
#include <cwchar>
#include <string>

namespace ts3::io {
namespace {
std::wstring userDocumentsDir;
// 0x00FB8350: all 16 entries, including the non-shell special cases.
constexpr int folders[16][2] = {
    {0,0}, {0,0}, {0,0}, {10,0}, {20,0}, {0,0}, {16,25}, {25,16},
    {26,35}, {35,26}, {5,46}, {46,5}, {13,53}, {53,13}, {2,23}, {23,2}
};
HRESULT ShellFolder(int folder, DWORD flags, wchar_t* path)
{
#if defined(_M_IX86)
    // Use the game's resolved import (also usable by differential fixtures).
    using Fn = HRESULT (WINAPI*)(HWND, int, HANDLE, DWORD, LPWSTR);
    return (*reinterpret_cast<Fn const*>(0x00F957BC))(nullptr, folder, nullptr, flags, path);
#else
    return SHGetFolderPathW(nullptr, folder, nullptr, flags, path);
#endif
}
}
void SetUserDocumentsDir(const wchar_t* path)
{
    userDocumentsDir = path ? path : L"";
}

// Original: TS3W.exe 0x004DC100, cdecl; four stack arguments, caller cleanup.
int SpecialDirectory(int folderId, wchar_t* path, char requireDirectory, uint32_t capacity)
{
    path[0] = 0;
    if (folderId == 1 || folderId == 2) {
        const DWORD result = folderId == 1 ? GetTempPathW(capacity, path)
                                          : GetWindowsDirectoryW(path, capacity);
        if (!result) return -1;
        // EA ignores this helper's return, including capacity exhaustion.
        CallGameFunction<unsigned char, 0x004DFF70>(path, capacity);
    } else if (folderId == 5) {
        wchar_t filename[260];
        wchar_t* filePart;
        if (!GetModuleFileNameW(GetModuleHandleA(nullptr), filename, 260) ||
            !GetFullPathNameW(filename, capacity, path, &filePart)) return -1;
        *filePart = 0;
    } else {
        if ((folderId == 10 || folderId == 11) && !userDocumentsDir.empty()) {
            // SHGetFolderPathW's contract is MAX_PATH, not the caller's capacity.
            if (userDocumentsDir.size() >= 260) return -1;
            std::wcscpy(path, userDocumentsDir.c_str());
        } else {
#if defined(_M_IX86)
            const int* pair = reinterpret_cast<const int*>(0x00FB8350) + folderId * 2;
#else
            const int* pair = folders[folderId];
#endif
            const int create = requireDirectory ? 0x8000 : 0;
            const DWORD flags = folderId == 3 ? 1 : 0;
            if (!pair[0] || ShellFolder(pair[0] | create, flags, path) < 0) {
                if (!pair[1] || ShellFolder(pair[1] | create, flags, path) < 0) return -1;
            }
        }
        const size_t length = std::wcslen(path);
        if (length && path[length - 1] != L'\\' && path[length - 1] != L'/' && path[length - 1] != L':') {
            if (length >= 259) return -1;
            path[length] = L'\\';
            path[length + 1] = 0;
        }
    }
    if (requireDirectory) {
        if (!*path) return -1;
        const DWORD attributes = GetFileAttributesW(path);
        if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY)) return -1;
    }
    return static_cast<int>(std::wcslen(path));
}
}
