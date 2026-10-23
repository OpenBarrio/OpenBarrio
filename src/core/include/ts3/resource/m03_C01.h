#pragma once
#include "ts3/resource/m03_A12.h"

namespace ts3::resource::m03::c01 {
using a12::Value; using a12::Iterator; using a12::KeyDeque; using a12::Preloader;
struct KeyList { Value* begin; Value* end; };
struct ParseContext { KeyList* target; const char16_t* prefix; uint32_t length; uint8_t enabled; };
struct Boundaries {
    void (*free)(void*);
    void (*clear)(KeyDeque*);
    uintptr_t (*insert)(KeyDeque*,Iterator,uint32_t,const Value*);
    void* (*parser)();
    void (*parse)(void*,void*,ParseContext*);
    void (*finish)(KeyList*);
    void* (*loader)();
    void (*cancel)(void*,void**);
    void (*release)(void*);
    void (*append)(a12::PointerDeque*,void*);
    void* (*factory)(uint32_t);
    bool (*load)(void*,const Value*,void**,Preloader*,unsigned);
};
const Boundaries& OriginalBoundaries();
// TS3W.exe 0x007305C0: splats iterator arguments into flat-source value copying; returns out.
Iterator* Assign(const Iterator*,Iterator* out,const Iterator* first,const Iterator* last,void* unused);
// TS3W.exe 0x00730630: copies a segmented value range backwards and returns out (donor shape, different callee).
Iterator* CopyBackward(Iterator* out,Iterator first,Iterator last,Iterator dst);
// TS3W.exe 0x00730740: processes flagged preload slots then consumes at most one pending key; incidental EAX is not a value.
void Tick(Preloader*,const Boundaries& = OriginalBoundaries());
// TS3W.exe 0x00730F90: copies backwards through scratch output; returns the numeric address of dead stack scratch (never dereference it).
uintptr_t ShiftBackward(const Iterator*,const Iterator* first,const Iterator* last,void* unused);
// TS3W.exe 0x00731760: parses a key list, then finalizes nonempty output; only AL is defined.
bool Parse(KeyList*,void* source,const char16_t* prefix,const Boundaries& = OriginalBoundaries());
// TS3W.exe 0x007317E0: erases a segmented value range, shifting the nearer side; preserves flat-source suffix quirk.
Iterator* Erase(KeyDeque*,Iterator* out,Iterator first,Iterator last,const Boundaries& = OriginalBoundaries());
// TS3W.exe 0x00731A20: grows with the supplied value or erases the suffix; returns insertion EAX on growth, dead stack scratch address on shrink.
uintptr_t Resize(KeyDeque*,uint32_t size,const Value*,const Boundaries& = OriginalBoundaries());
#if defined(_M_IX86)
inline Iterator* __fastcall HookAssign(const Iterator* p,void*,Iterator* out,const Iterator* a,const Iterator* b,void* u) { return Assign(p,out,a,b,u); }
inline void __fastcall HookTick(Preloader* p,void*) { Tick(p); }
inline uintptr_t __fastcall HookShiftBackward(const Iterator* p,void*,const Iterator* a,const Iterator* b,void* u) { return ShiftBackward(p,a,b,u); }
inline bool __fastcall HookParse(KeyList* p,void*,void* src,const char16_t* prefix) { return Parse(p,src,prefix); }
inline Iterator* __fastcall HookErase(KeyDeque* p,void*,Iterator* out,Iterator a,Iterator b) { return Erase(p,out,a,b); }
inline uintptr_t __fastcall HookResize(KeyDeque* p,void*,uint32_t n,const Value* v) { return Resize(p,n,v); }
#endif
}
