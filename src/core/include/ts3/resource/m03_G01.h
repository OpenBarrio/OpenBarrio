#pragma once
#include "ts3/resource/m03_B01.h"
namespace ts3::resource::m03::g01 {
using Pair=b01::Pair;
using Vector=b01::Vector;
using Hash=b01::Hash;
using Node=b01::Node;
// Used-field view only; padding denotes untouched manager subobjects.
struct Manager {
    void** vtable;
    uint32_t initialized;
    allocator::ICoreAllocator* allocator;
    uint8_t reserved0c[0x24];
    Vector databases;
    uint8_t databaseMutex[0x30];
    Hash factories;
    uint8_t reservedA0[0x100];
    uint8_t factoryMutex[0x30];
};
TS3_X86_OFFSET(Manager,databases,0x30);
TS3_X86_OFFSET(Manager,databaseMutex,0x48);
TS3_X86_OFFSET(Manager,factories,0x78);
TS3_X86_OFFSET(Manager,factoryMutex,0x1a0);
// TS3W.exe 0x004B29D0: register factories by type or remove every occurrence; duplicate registrations are retained. Only AL is contractual.
uint8_t __fastcall Factory(Manager*,void*,uint32_t,void*,int32_t);
// TS3W.exe 0x004B2D00: add/remove a priority-ordered database, unlocking around callbacks; preserve unlock high EAX and callback AL.
uint32_t __fastcall Database(Manager*,void*,uint32_t,void*,int32_t);
// TS3W.exe 0x004B2EC0: reposition a database using requested priority but reinsert aliased old storage (EA bug), returning unlock EAX.
uint32_t __fastcall Priority(Manager*,void*,void*,int32_t);
}
