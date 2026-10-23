#pragma once
#include <cstdint>
namespace ts3::resource::b05 {
// Native hosts inject the unresolved scripting/locale/allocation boundaries.
struct Boundaries {
    void* (*script)();
    void* (*locale)();
    uintptr_t (*lookup)(void*,void*,bool);
    uintptr_t (*tail)(unsigned,uintptr_t);
    void* (*allocate)();
    const void* dataTable;
    const void* localeTable;
    const void* dataName;
    const void* dataVtable;
    const void* localeVtable;
};
extern thread_local const Boundaries* boundaries;
// TS3W.exe 0x0073FFC0: dispatches factory class-name lookup through scripting slot 95 then slot 18.
uintptr_t __cdecl DataClassName(uintptr_t arg);
// TS3W.exe 0x0073FFE0: dispatches factory assembly-name lookup through scripting slot 95 then slot 19.
uintptr_t __cdecl DataAssemblyName(uintptr_t arg);
// TS3W.exe 0x00740000: writes the DataFactory binding table and returns the output address.
const void** __fastcall DataTable(void*,void*,const void** out);
// TS3W.exe 0x00740010: returns the DataFactory registration name.
const void* __fastcall DataName(void*,void*);
// TS3W.exe 0x00740020: allocates and registers a DataFactory adapter, including null on failure.
uintptr_t __cdecl RegisterData(void* service);
// TS3W.exe 0x007437B0: converts a managed string then performs UInt64 locale lookup.
uintptr_t __cdecl LocaleUInt64(uintptr_t arg);
// TS3W.exe 0x007437E0: converts a managed string then performs string locale lookup.
uintptr_t __cdecl LocaleString(uintptr_t arg);
// TS3W.exe 0x00743810: forwards to the locale update boundary, preserving its raw return.
uintptr_t __cdecl LocaleUpdate(uintptr_t arg);
// TS3W.exe 0x00743820: forwards to the change-locale binding boundary.
uintptr_t __cdecl ChangeLocale();
// TS3W.exe 0x00743830: forwards to the get-locale binding boundary.
uintptr_t __cdecl GetLocale();
// TS3W.exe 0x00743840: forwards to the localized-logo suffix binding boundary.
uintptr_t __cdecl LogoSuffix();
// TS3W.exe 0x00743870: writes the locale binding table and returns the output address.
const void** __fastcall LocaleTable(void*,void*,const void** out);
// TS3W.exe 0x00743880: allocates and registers a locale adapter, including null on failure.
uintptr_t __cdecl RegisterLocale(void* service);
}
