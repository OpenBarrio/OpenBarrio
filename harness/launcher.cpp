// OpenBarrio suspended-process launcher.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdio>
#include <string>

int wmain(int argc, wchar_t** argv)
{
    if (argc < 2) {
        std::fwprintf(stderr, L"usage: openbarrio_launch <TS3W.exe> [harness.dll] [game args...]\n");
        return 2;
    }
    const std::wstring exe = argv[1];
    std::wstring dll;
    int firstArg = 2;
    if (argc > 2 && std::wstring(argv[2]).size() > 4 && _wcsicmp(argv[2] + wcslen(argv[2]) - 4, L".dll") == 0) {
        dll = argv[2];
        firstArg = 3;
    } else {
        wchar_t self[MAX_PATH];
        GetModuleFileNameW(nullptr, self, MAX_PATH);
        dll = self;
        dll = dll.substr(0, dll.find_last_of(L"\\/") + 1) + L"openbarrio_test.dll";
    }
    wchar_t full[MAX_PATH];
    GetFullPathNameW(dll.c_str(), MAX_PATH, full, nullptr);
    dll = full;
    if (GetFileAttributesW(dll.c_str()) == INVALID_FILE_ATTRIBUTES) {
        std::fwprintf(stderr, L"harness not found: %ls\n", dll.c_str());
        return 2;
    }

    std::wstring cmd = L"\"" + exe + L"\"";
    for (int i = firstArg; i < argc; ++i)
        cmd += L" " + std::wstring(argv[i]);
    const std::wstring dir = exe.substr(0, exe.find_last_of(L"\\/"));

    STARTUPINFOW si{sizeof si};
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(exe.c_str(), cmd.data(), nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, dir.c_str(), &si, &pi)) {
        std::fwprintf(stderr, L"CreateProcess failed: %lu\n", GetLastError());
        return 1;
    }

    
    
    
    const size_t bytes = (dll.size() + 1) * sizeof(wchar_t);
    void* remote = VirtualAllocEx(pi.hProcess, nullptr, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    bool ok = remote && WriteProcessMemory(pi.hProcess, remote, dll.c_str(), bytes, nullptr);
    DWORD loaded = 0;
    if (ok) {
        auto* loadLibrary = reinterpret_cast<LPTHREAD_START_ROUTINE>(
            GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW"));
        HANDLE t = CreateRemoteThread(pi.hProcess, nullptr, 0, loadLibrary, remote, 0, nullptr);
        ok = t != nullptr;
        if (t) {
            WaitForSingleObject(t, 60000);
            GetExitCodeThread(t, &loaded);
            CloseHandle(t);
        }
    }
    if (!ok || loaded == 0) {
        std::fwprintf(stderr, L"injecting %ls failed (error %lu); stopping the game\n", dll.c_str(), GetLastError());
        TerminateProcess(pi.hProcess, 1);
        return 1;
    }
    VirtualFreeEx(pi.hProcess, remote, 0, MEM_RELEASE);
    ResumeThread(pi.hThread);
    std::fwprintf(stdout, L"started TS3W.exe (pid %lu) with %ls\n", pi.dwProcessId, dll.c_str());
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 0;
}
