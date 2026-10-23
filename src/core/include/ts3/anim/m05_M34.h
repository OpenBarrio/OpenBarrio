#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim {
struct M34Clip { Controller base; void* listener; Controller* first; Controller* second; };
struct M34Array { void* begin; void* end; void* capacity; void* descriptor; };
struct M34Orient { Controller base; float time; M34Array orient; M34Array rotation; M34Array rotor; };
struct M34Event { Controller* source; ResourcePrefix* resource; void* host; uint32_t a,b; uint8_t x,y,z; };
TS3_X86_OFFSET(M34Clip,listener,0x28);
TS3_X86_OFFSET(M34Orient,orient,0x2c);
TS3_X86_OFFSET(M34Orient,rotor,0x4c);
// TS3W.exe 0x0056A5F0: Detach the clip listener and release second then first, pinning last references to one.
void M34Destruct(M34Clip*);
// TS3W.exe 0x0056A670: Forward a copied event with source and host replaced, retaining across callbacks.
void M34Forward(void*,const M34Event*,uint32_t);
// TS3W.exe 0x0056A730: Destruct and optionally free, returning this even after free.
M34Clip* M34Delete(M34Clip*,uint32_t);
// TS3W.exe 0x0056AA70: Construct the orientation controller and its empty orientation array.
M34Orient* M34Construct(M34Orient*,void*);
// TS3W.exe 0x0056AAF0: Adjust the secondary interface by minus 0xC and tail-call the deleting destructor.
void* M34DeleteSecondary(void*,uint32_t);
// TS3W.exe 0x0056ABB0: Construct the rotor controller and three empty arrays, preserving opaque bytes.
M34Orient* M34ConstructRotor(M34Orient*,void*);
struct M34Hooks {
 void Destruct(){M34Destruct(reinterpret_cast<M34Clip*>(this));}
 void Forward(void* e,uint32_t u){M34Forward(this,static_cast<const M34Event*>(e),u);}
 void* Delete(uint32_t f){return M34Delete(reinterpret_cast<M34Clip*>(this),f);}
 void* Construct(void* h){return M34Construct(reinterpret_cast<M34Orient*>(this),h);}
 void* DeleteSecondary(uint32_t f){return M34DeleteSecondary(this,f);}
 void* ConstructRotor(void* h){return M34ConstructRotor(reinterpret_cast<M34Orient*>(this),h);}
};
}
