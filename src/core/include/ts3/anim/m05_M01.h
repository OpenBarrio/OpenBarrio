#pragma once
#include "ts3/anim/m05_B01.h"
#include "ts3/anim/m05_B02.h"
namespace ts3::anim::m01 {
using Registry=b02::Registry;
using Node=b01::Node;
using Cursor=b01::Cursor;
// TS3W.exe 0x0054C510: Erase a global factory key, replacing only AL with one.
uintptr_t EraseFactory(uint32_t key);
// TS3W.exe 0x0054C580: Serialize the current evaluator, preserving the payload return's upper EAX bytes.
uintptr_t SaveCurrent(Registry*,Cursor*);
// TS3W.exe 0x0054C5A0: Deserialize the current evaluator, preserving the payload return's upper EAX bytes.
uintptr_t LoadCurrent(Registry*,Cursor*);
// TS3W.exe 0x0054C5E0: Release registered evaluators in list order, recycle nodes, and reset the workspace heap.
uintptr_t Clear(Registry*);
// TS3W.exe 0x0054C6E0: Clear the registry then destroy its workspace heap.
uintptr_t Destroy(Registry*);
// TS3W.exe 0x0054C7F0: Construct an evaluator base then install the derived table; sibling links stay untouched.
Node* Construct(Node*,void* host);
}
#if defined(_M_IX86)
namespace ts3::anim::m01::hooks {
uintptr_t __cdecl EraseFactory(uint32_t);
uintptr_t __fastcall SaveCurrent(Registry*,void*,Cursor*);
uintptr_t __fastcall LoadCurrent(Registry*,void*,Cursor*);
uintptr_t __fastcall Clear(Registry*,void*);
uintptr_t __fastcall Destroy(Registry*,void*);
Node* __fastcall Construct(Node*,void*,void*);
}
#endif
