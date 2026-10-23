#pragma once
#include "ts3/layout.h"
namespace ts3::ui {
// Sparse native view of the fields accessed by this batch. The receiver of
// interface entries is &secondary, not the primary window.
struct B12Window {
    void* primary; void* secondary;
    void* next; void* previous;
    unsigned char opaque10[0x18];
    uint32_t word28; B12Window* ancestor; void* manager;
    unsigned char opaque34[0xc];
    float rect[12];
    uint32_t flags;
    uint32_t interfaceWord;
    unsigned char opaque78[0x180];
    uint8_t dirty,clip,pad[2];
    uint32_t opaque1fc;
    uint32_t index; void* value; void* opaque208; void* resource;
    struct { void* next; void* previous; } children;
};
TS3_X86_OFFSET(B12Window,manager,0x30);
TS3_X86_OFFSET(B12Window,dirty,0x1f8);
TS3_X86_OFFSET(B12Window,resource,0x20c);
TS3_X86_OFFSET(B12Window,children,0x210);
// TS3W.exe 0x00513BB0: store interface-relative +70 and notify the manager.
void B12SetWord(void* self,uint32_t value);
// TS3W.exe 0x00513BD0: replace a retained resource and invalidate on change.
void B12SetResource(void* self,void* resource);
// TS3W.exe 0x00513C20: notify the manager of an interface update.
void B12Notify(void* self);
// TS3W.exe 0x00513C40: restore rectangles recursively, calling the adjusted interface.
void B12Restore(B12Window* self);
// TS3W.exe 0x00513C90: send the alternate manager notification.
void B12NotifyAlternate(void* self);
// TS3W.exe 0x00513CB0: notify eligible descendants; query each manager twice.
void B12Broadcast(B12Window* self,void* payload);
// TS3W.exe 0x00513D30: set dirty bits up the ancestor chain and notify once.
void B12Dirty(void* self);
// TS3W.exe 0x00513D80: dispatch message 0x10003 through the interface.
void B12Dispatch(void* self);
// TS3W.exe 0x00513DC0: propagate inherited state only into changed subtrees.
void B12Propagate(B12Window* self);
// TS3W.exe 0x00513E40: return the interface-relative +200 pointer (verified getter twin).
void* B12GetValue(void* self);
// TS3W.exe 0x00513E50: set the interface index, invalidate, then send message 14/4.
void B12SetIndex(void* self,uint32_t value);
// TS3W.exe 0x00513EB0: inclusive clipped hit test; unordered comparisons follow COMISS.
bool B12Hit(void* self,float x,float y);
// TS3W.exe 0x00513FD0: map a point virtually, zeroing output when AL is false.
float* B12Map(void* self,float* output,float x,float y);
// ABI-only facade: this is the adjusted interface address, never an allocation.
struct B12Hooks {
    void B12SetWord(uint32_t v) { ts3::ui::B12SetWord(this,v); }
    void B12SetResource(void* v) { ts3::ui::B12SetResource(this,v); }
    void B12Notify() { ts3::ui::B12Notify(this); }
    void B12NotifyAlternate() { ts3::ui::B12NotifyAlternate(this); }
    void B12Dirty() { ts3::ui::B12Dirty(this); }
    void B12Dispatch() { ts3::ui::B12Dispatch(this); }
    void* B12GetValue() { return ts3::ui::B12GetValue(this); }
    void B12SetIndex(uint32_t v) { ts3::ui::B12SetIndex(this,v); }
    bool B12Hit(float x,float y) { return ts3::ui::B12Hit(this,x,y); }
    void* B12Map(void* out,float x,float y) { return ts3::ui::B12Map(this,static_cast<float*>(out),x,y); }
};
}
