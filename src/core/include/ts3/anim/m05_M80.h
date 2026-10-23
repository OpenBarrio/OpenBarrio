#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m80 {
struct Text { char *begin,*end,*capacity; unsigned char opaque[4]; char storage[96]; };
struct Record { Text first,second; uint32_t data[8]; int32_t index; void *key,*target; unsigned char tail[4]; };
template<class T> struct Vector { T *begin,*end,*capacity; void* descriptor; };
struct Group { void* header[2]; Vector<Record> records; unsigned char rest[2720]; };
// Only the consumed prefix is modeled; opaque portions are not initialized.
struct State { unsigned char prefix[144]; Group *begin,*end,*capacity; void* descriptor; Vector<Text> names; Vector<void*> keys; uint32_t unknown[2]; void* enabled; uint32_t flags; };
TS3_X86_SIZE(Text,112); TS3_X86_SIZE(Record,272); TS3_X86_SIZE(Group,2744);
TS3_X86_OFFSET(State,names,160); TS3_X86_OFFSET(State,enabled,200);
// TS3W.exe 0x00578090: Insert one inline string, preserving EA alias adjustment.
uintptr_t InsertText(Vector<Text>*,Text*,const Text*);
// TS3W.exe 0x00578200: Insert records; EA growth deliberately has no allocation.
uintptr_t InsertRecords(Vector<Record>*,Record*,uint32_t,const Record*);
// TS3W.exe 0x005783F0: Insert repeated inline strings with geometric growth.
uintptr_t InsertTexts(Vector<Text>*,Text*,uint32_t,const Text*);
// TS3W.exe 0x00578600: Append an empty inline string without clearing opaque bytes.
uintptr_t AppendText(Vector<Text>*);
// TS3W.exe 0x00578650: Update an IK binding and queue its key, returning only AL.
bool Bind(State*,uint32_t,uint32_t,const uint32_t*,void*,void*);
// TS3W.exe 0x005787C0: Resize the record vector, preserving EA shrink behavior.
uintptr_t ResizeRecords(Vector<Record>*,uint32_t);
}

#if defined(_M_IX86)
namespace ts3::anim::m80::hooks {
uintptr_t __fastcall InsertText(Vector<Text>* v, void*, Text* p, const Text* q);
uintptr_t __fastcall InsertRecords(Vector<Record>* v, void*, Record* p, uint32_t n, const Record* q);
uintptr_t __fastcall InsertTexts(Vector<Text>* v, void*, Text* p, uint32_t n, const Text* q);
uintptr_t __fastcall AppendText(Vector<Text>* v, void*);
bool __fastcall Bind(State* s, void*, uint32_t g, uint32_t i, const uint32_t* r, void* k, void* t);
uintptr_t __fastcall ResizeRecords(Vector<Record>* v, void*, uint32_t n);
}
#endif
