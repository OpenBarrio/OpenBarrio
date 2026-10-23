#pragma once
#include "ts3/ui/cls_4a4230_UI.h"
namespace ts3::ui::m08 { struct Vector2; }
namespace ts3::ui::m08::b14 {
// Sparse native view of the accessed window fields; holes are not new ownership claims.
struct Link { Link* next; Link* previous; };
struct Window;
struct ValueNode { Link link; void* value; };
struct MapNode { uint32_t key; void* value; MapNode* next; };
struct Map { void* table; MapNode** buckets; uint32_t count; };
struct Window {
    void* primary; void* secondary; Link sibling;
    uint8_t hole010[0x18]; uint32_t refs;
    Window* parent; void* manager; uint8_t hole034[0xc];
    float rect[4]; uint8_t hole050[0x20]; uint32_t flags;
    uint8_t hole074[0x3c]; float transform[16]; uint8_t hole0F0[0xb0];
    float inverse[16]; uint32_t inverseKind; uint8_t hole1E4[0x10];
    uint32_t opaque1F4; uint32_t state; uint32_t index; void* resource; void* service; void* hole208; void* hole20C;
    Link children; Link values; void* interface220; Map map;
};
TS3_X86_OFFSET(Window,refs,0x28);
TS3_X86_OFFSET(Window,parent,0x2c);
TS3_X86_OFFSET(Window,rect,0x40);
TS3_X86_OFFSET(Window,transform,0xb0);
TS3_X86_OFFSET(Window,inverse,0x1a0);
TS3_X86_OFFSET(Window,state,0x1f8);
TS3_X86_OFFSET(Window,resource,0x200);
TS3_X86_OFFSET(Window,children,0x210);
TS3_X86_OFFSET(Window,map,0x224);
inline Window* Owner(void* secondary) { return reinterpret_cast<Window*>(static_cast<uint8_t*>(secondary)-offsetof(Window,secondary)); }
inline Window* FromLink(Link* p) { return reinterpret_cast<Window*>(reinterpret_cast<uint8_t*>(p)-offsetof(Window,sibling)); }
// TS3W.exe 0x00514B60: replace the retained resource and notify the manager.
void SetResource(void* self,void* resource);
// TS3W.exe 0x00514C20: transform a local point through the ancestor chain, returning out.
Vector2* ToScreen(void* self,Vector2* out,float x,float y);
// TS3W.exe 0x00514DB0: inverse-project a point; singular kind leaves output untouched.
uint8_t FromScreen(void* self,float x,float y,Vector2* out);
// TS3W.exe 0x00514F30: return the first child link through out, returning out.
Link** First(void* self,Link** out);
// TS3W.exe 0x00514F40: return the child sentinel through out, returning out.
Link** End(void* self,Link** out);
// TS3W.exe 0x00514F50: get a child's link or the virtual sentinel, returning out; null child faults in EA.
Link** FindChild(void* self,Link** out,void* child);
// TS3W.exe 0x00514FA0: retain and attach a child before the first non-topmost child, rolling back failed init.
uint8_t Attach(void* self,void* child);
// TS3W.exe 0x00515040: move a child to the front of its topmost partition.
void BringFront(void* self,void* child);
// TS3W.exe 0x005150B0: move a child to the end of its topmost partition; notifies even on no-op.
void SendBack(void* self,void* child);
// TS3W.exe 0x00515140: return the value after a matching value, or first for null.
void* NextValue(void* self,void* value);
// TS3W.exe 0x00515180: dispatch the window's selected message types, preserving AL returns.
uint8_t Message(Window* self,const GridMessage* message);
// TS3W.exe 0x00515410: look up a key in the window hash map.
void* Lookup(void* self,uint32_t key);
// TS3W.exe 0x00515C30: adjust secondary receiver and atomically increment the primary reference count.
uint32_t Retain(void* self);
#if defined(_M_IX86)
inline void __fastcall HookSetResource(void* s,void*,void* r) { return SetResource(s,r); }
inline Vector2* __fastcall HookToScreen(void* s,void*,Vector2* o,float x,float y) { return ToScreen(s,o,x,y); }
inline uint8_t __fastcall HookFromScreen(void* s,void*,float x,float y,Vector2* o) { return FromScreen(s,x,y,o); }
inline Link** __fastcall HookFirst(void* s,void*,Link** o) { return First(s,o); }
inline Link** __fastcall HookEnd(void* s,void*,Link** o) { return End(s,o); }
inline Link** __fastcall HookFindChild(void* s,void*,Link** o,void* c) { return FindChild(s,o,c); }
inline uint8_t __fastcall HookAttach(void* s,void*,void* c) { return Attach(s,c); }
inline void __fastcall HookBringFront(void* s,void*,void* c) { return BringFront(s,c); }
inline void __fastcall HookSendBack(void* s,void*,void* c) { return SendBack(s,c); }
inline void* __fastcall HookNextValue(void* s,void*,void* v) { return NextValue(s,v); }
inline uint8_t __fastcall HookMessage(Window* s,void*,const GridMessage* m) { return Message(s,m); }
inline void* __fastcall HookLookup(void* s,void*,uint32_t k) { return Lookup(s,k); }
inline uint32_t __fastcall HookRetain(void* s,void*) { return Retain(s); }
#endif
}
