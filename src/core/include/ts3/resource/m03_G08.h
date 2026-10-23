#pragma once
#include "ts3/resource/m03_D03.h"
#include "ts3/layout.h"
namespace ts3::resource::g08 {
using Locale=d03::Locale;
struct String { char16_t* begin; char16_t* end; char16_t* capacity; const char* tag; };
struct Entry { Entry* next; uint32_t reserved,key[2]; const char16_t* text; };
struct Table {
 void* vtable; int32_t refs; uint32_t base[8];
 Entry* buckets[7919]; Entry* sentinel; uint32_t count,last;
 void* data; void* strings;
};
TS3_X86_OFFSET(Table,data,0x7bf0);
TS3_X86_OFFSET(Table,strings,0x7bf4);
// TS3W.exe 0x0059C430: change locale, rebuild tables and notify listeners; unchanged locale returns shifted ID.
uintptr_t __fastcall ChangeLocale(Locale*,void*,const char16_t*);
// TS3W.exe 0x0059C5B0: return a managed localized string, applying Japanese fallback substitutions.
uint32_t __fastcall ManagedLookup(Locale*,void*,uint32_t,uint32_t);
// TS3W.exe 0x0059CB00: copy localized text and expand escapes; second escape search starts at -1 as in EA.
uint8_t __fastcall TextLookup(Locale*,void*,uint32_t,uint32_t,String*);
// TS3W.exe 0x0059F6C0: release both backing buffers then clear buckets/count, preserving sentinel and last.
void* __fastcall ClearTable(Table*,void*);
// TS3W.exe 0x0059F7C0: construct a table, leaving its last-bucket word untouched and sentinel at -1.
Table* __fastcall ConstructTable(Table*,void*);
}
