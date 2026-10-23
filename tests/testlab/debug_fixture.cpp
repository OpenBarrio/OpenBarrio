#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cwchar>
int wmain(int argc,wchar_t** argv) {
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    if (argc<2) return 2;
    if (!std::wcscmp(argv[1],L"handled")) {
        __try { RaiseException(0xE0421001,0,0,nullptr); }
        __except(EXCEPTION_EXECUTE_HANDLER) { return 0; }
    }
    if (!std::wcscmp(argv[1],L"crash")) RaiseException(0xE0421001,EXCEPTION_NONCONTINUABLE,0,nullptr);
    return 7;
}
