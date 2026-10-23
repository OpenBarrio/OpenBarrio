#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::audio {
struct B48Point { float x,y; };
struct B48Segment { float x,y,previousX,previousY,slope; };
struct B48Curve {
    B48Segment *begin,*end,*capacity;
    const char* label;
    B48Segment* storage;
    B48Segment inlineData[42];
    uint32_t count;
};
// Word-only sound view; the retained object at x86 +100 is accessed separately.
struct B48Sound { uint8_t prefix[0x38]; uint32_t key[2],value; uint8_t pad[4]; uint32_t record[4]; uint8_t tail[0xADC-0x58]; uint32_t flags; };
// TS3W.exe 0x008AAAD0: Build response segments; signed dy threshold leaves upward vertical entries unchanged but divides downward ones by zero.
bool B48Build(B48Curve*,const B48Point*,int32_t);
// TS3W.exe 0x008AAEF0: Reset to identity and load optional resource point pairs.
bool B48Load(B48Curve*,void*,uint32_t);
// TS3W.exe 0x008AAF70: Initialize inline response storage and identity points.
B48Curve* B48Construct(B48Curve*);
// TS3W.exe 0x008AAFE0: Parse float pairs; auto delimiter is first non-alphanumeric byte, and tokens truncate at 127 bytes.
bool B48Parse(B48Curve*,const char*,char);
// TS3W.exe 0x008AB1E0: Compute the ratio of equal-temperament frequencies using EA constants.
double B48Ratio(void*,int32_t,int32_t);
// TS3W.exe 0x008AB240: Assign a retained resource, adding before publishing before releasing.
void B48Assign(void*,void*);
// TS3W.exe 0x008AB250: Return the embedded resource view.
void* B48Resource(void*);
// TS3W.exe 0x008AB260: Store two key words and return the first word.
uint32_t B48SetKey(B48Sound*,uint32_t,uint32_t);
// TS3W.exe 0x008AB280: Return the two key words in EDX:EAX.
uint64_t B48GetKey(B48Sound*);
// TS3W.exe 0x008AB290: Store the value and set dirty bit two.
uint32_t B48SetValue(B48Sound*,uint32_t);
// TS3W.exe 0x008AB2B0: Return the stored value.
uint32_t B48GetValue(B48Sound*);
// TS3W.exe 0x008AB2C0: Return dirty state in AL, retaining the high flags bytes in EAX.
uint32_t B48Dirty(B48Sound*);
// TS3W.exe 0x008AB2E0: Copy the four-word record in original alias-sensitive order.
uint32_t* B48Copy(B48Sound*,uint32_t*);
// TS3W.exe 0x008AB300: Return the word at offset 33C.
uint32_t B48Word(void*);
}

#if defined(_M_IX86)
namespace ts3::audio {
inline bool __fastcall B48BuildAbi(B48Curve* self,void*,const B48Point* p,int32_t n) { return B48Build(self,p,n); }
inline bool __fastcall B48LoadAbi(B48Curve* self,void*,void* r,uint32_t id) { return B48Load(self,r,id); }
inline B48Curve* __fastcall B48ConstructAbi(B48Curve* self,void*) { return B48Construct(self); }
inline bool __fastcall B48ParseAbi(B48Curve* self,void*,const char* s,char d) { return B48Parse(self,s,d); }
inline double __fastcall B48RatioAbi(void* self,void*,int32_t a,int32_t b) { return B48Ratio(self,a,b); }
inline void __fastcall B48AssignAbi(void* self,void*,void* r) { return B48Assign(self,r); }
inline void* __fastcall B48ResourceAbi(void* self,void*) { return B48Resource(self); }
inline uint32_t __fastcall B48SetKeyAbi(B48Sound* self,void*,uint32_t a,uint32_t b) { return B48SetKey(self,a,b); }
inline uint64_t __fastcall B48GetKeyAbi(B48Sound* self,void*) { return B48GetKey(self); }
inline uint32_t __fastcall B48SetValueAbi(B48Sound* self,void*,uint32_t v) { return B48SetValue(self,v); }
inline uint32_t __fastcall B48GetValueAbi(B48Sound* self,void*) { return B48GetValue(self); }
inline uint32_t __fastcall B48DirtyAbi(B48Sound* self,void*) { return B48Dirty(self); }
inline uint32_t* __fastcall B48CopyAbi(B48Sound* self,void*,uint32_t* o) { return B48Copy(self,o); }
inline uint32_t __fastcall B48WordAbi(void* self,void*) { return B48Word(self); }
}
#endif
