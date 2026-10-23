#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m96 {
struct Entity {
    void** vtable;
    int32_t refs;
    void** secondary;
    uint32_t flags;
    uint32_t idLow,idHigh;
    void* cached;
};
struct Identifier { void* opaque; uint32_t tag; uint32_t low,high; };
struct Context { void* opaque; void* nameSpace; void* provider; void* fallback; };
struct Handle { Context* context; };
struct Stream { void* opaque[2]; void* buffer; uintptr_t reserved; uint32_t mode; };
TS3_X86_OFFSET(Entity,idLow,0x10);
TS3_X86_OFFSET(Entity,cached,0x18);
TS3_X86_OFFSET(Context,provider,8);
TS3_X86_OFFSET(Stream,mode,0x10);
// TS3W.exe 0x00581F40: invalidate cached state only when either identifier word changes; EAX is unspecified.
void SetIdentifier(Entity*,uint32_t,uint32_t);
// TS3W.exe 0x00581F70: copy identifier words or clear them for a null source; EAX is unspecified.
void CopyIdentifier(Entity*,const Identifier*);
// TS3W.exe 0x00582120: deserialize named optional scene entities in order, retaining partial work on failure.
bool LoadEntities(void*,Stream*,void*);
// TS3W.exe 0x00582660: prefer the provider's slot 9 result, otherwise query the fallback.
void* ResolveRig(Handle*);
// TS3W.exe 0x005826D0: resolve a joint attribute and emit EA's 1/2/4/5 failure codes.
uint32_t ResolveJoint(void*,uint32_t,uint32_t*);
// TS3W.exe 0x00582760: look up a scene entity using the fallback name or "unknown_namespace".
void* FindEntity(Handle*);
}
#if defined(_M_IX86)
namespace ts3::anim::m96::hooks {
inline void __fastcall SetIdentifier(Entity* s,void*,uint32_t a,uint32_t b){m96::SetIdentifier(s,a,b);}
inline void __fastcall CopyIdentifier(Entity* s,void*,const Identifier* a){m96::CopyIdentifier(s,a);}
inline bool __fastcall LoadEntities(void* s,void*,Stream* a,void* b){return m96::LoadEntities(s,a,b);}
inline void* __fastcall ResolveRig(Handle* s,void*){return m96::ResolveRig(s);}
inline void* __fastcall FindEntity(Handle* s,void*){return m96::FindEntity(s);}
}
#endif
