#include "ts3/deviceconfig/clu_740fc0_DeviceConfig.h"
#include "ts3/layout.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
namespace ts3::deviceconfig {
namespace {
// Indirect through the game's import table: exehost can supply deterministic registry
// fakes and the harness uses exactly the imports used by the original executable.
template<class T> T Import(unsigned address) {
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(static_cast<uintptr_t>(address));
#else
    (void)address; AbortNotRewritten("DeviceConfig registry import"); return {};
#endif
}
const char* Key() { return Import<const char*>(0x11634B0); }
const wchar_t* Value() { return Import<const wchar_t*>(0x11634B8); }
}
// 0x007D04A0: Type is deliberately not validated (EA accepts any four-byte value).
int32_t ReadTelemetry() {
    HKEY key; int32_t data; DWORD size=4, type;
    int32_t result=0;
    if (!Import<decltype(&RegOpenKeyExA)>(0xF95090)(HKEY_LOCAL_MACHINE, Key(), 0, 0x20019, &key)) {
        if (!Import<decltype(&RegQueryValueExW)>(0xF95050)(key,Value(),nullptr,&type,reinterpret_cast<BYTE*>(&data),&size)) result=data;
        Import<decltype(&RegCloseKey)>(0xF95088)(key);
    }
    return result;
}
// 0x007D0510: Set/close errors are ignored, as in EA.
uint8_t WriteTelemetry(int32_t data) {
    HKEY key;
    if (Import<decltype(&RegOpenKeyExA)>(0xF95090)(HKEY_LOCAL_MACHINE,Key(),0,0x20006,&key)
        && Import<decltype(&RegCreateKeyA)>(0xF95048)(HKEY_LOCAL_MACHINE,Key(),&key)) return 0;
    Import<decltype(&RegSetValueExW)>(0xF9504C)(key,Value(),0,4,reinterpret_cast<const BYTE*>(&data),4);
    Import<decltype(&RegCloseKey)>(0xF95088)(key);
    return 1;
}
// 0x00740FC0
int32_t AuthenticateDisc() { return ReadTelemetry()!=0; }
// 0x00740FD0: Only the low byte of the argument is tested, before tailcalling.
uint8_t SetRegistryTelemetry(uint8_t enabled) { return WriteTelemetry(enabled!=0); }
}
