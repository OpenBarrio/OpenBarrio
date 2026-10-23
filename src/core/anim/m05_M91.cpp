#include "ts3/anim/m05_M91.h"
#include <cstring>
#include <algorithm>
namespace ts3::anim::m91 {
namespace {
uint32_t Result(uint32_t r,bool b) { return (r&0xffffff00u)|uint32_t(b); }
uint32_t Compare(const char* a,const char* b) {
 while(*a && static_cast<unsigned char>(*a)==static_cast<unsigned char>(*b)) { ++a;++b; }
 return static_cast<unsigned char>(*a)==static_cast<unsigned char>(*b)?0:static_cast<unsigned char>(*a)<static_cast<unsigned char>(*b)?0xffffffffu:1;
}
}
uint8_t __fastcall ReadText(TextEvent* s,void*,uint32_t version,uint8_t** p,uint32_t* n,uint32_t) {
 if(version<0x103 || *n<128) return 0;
 const size_t len=std::strlen(reinterpret_cast<char*>(*p));
 const size_t copied=std::min(len,size_t(127));
 std::memcpy(s->text,*p,copied);s->text[copied]=0;
 *p+=128;*n-=32;return 1;
}
uint8_t __fastcall ReadKey(KeyEvent* s,void*,uint32_t,uint8_t** p,uint32_t* n,uint32_t swap) {
 if(*n<16) return 0;
 if(uint8_t(swap)) for(unsigned i=0;i<16;i+=4) { std::swap((*p)[i],(*p)[i+3]);std::swap((*p)[i+1],(*p)[i+2]); }
 std::memcpy(&s->value,*p,4);std::memcpy(&s->keyLow,*p+8,4);std::memcpy(&s->keyHigh,*p+12,4);
 s->tagLow=0x02dc343f;s->tagHigh=0;*p+=16;*n-=4;return 1;
}
uint32_t __fastcall EqualText(TextEvent* s,void*,const TextEvent* b) {
 uint32_t r=b25::Equal(&s->base,nullptr,&b->base);if(!uint8_t(r)) return r;
 r=Compare(s->text,b->text);return Result(r,r==0);
}
uint32_t __fastcall EqualKey(KeyEvent* s,void*,const KeyEvent* b) {
 uint32_t r=b25::Equal(&s->base,nullptr,&b->base);if(!uint8_t(r)) return Result(r,false);
 r=s->value;if(r!=b->value)return Result(r,false);
 if(s->keyLow!=b->keyLow)return Result(r,false);
 r=s->tagLow;if(r!=b->tagLow)return Result(r,false);
 if(s->tagHigh!=b->tagHigh)return Result(r,false);
 r=s->keyHigh;return Result(r,r==b->keyHigh);
}
uint32_t __fastcall EqualWord(WordEvent* s,void*,const WordEvent* b) {
 uint32_t r=b25::Equal(&s->base,nullptr,&b->base);if(!uint8_t(r))return r;
 return Result(s->value,s->value==b->value);
}
uint32_t __fastcall EqualNamed(NamedEvent* s,void*,const NamedEvent* b) {
 uint32_t r=b25::Equal(&s->base,nullptr,&b->base);if(!uint8_t(r))return Result(r,false);
 r=s->fields[0];if(r!=b->fields[0]||s->fields[1]!=b->fields[1])return Result(r,false);
 r=s->fields[2];if(r!=b->fields[2]||s->fields[3]!=b->fields[3])return Result(r,false);
 r=s->fields[4];if(r!=b->fields[4])return Result(r,false);
 // Pointer-sized native field; only x86's low register bits are returned.
 r=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(s->name));
 if(bool(s->name)!=bool(b->name))return Result(r,false);
 if(s->name && b->name) { r=Compare(s->name,b->name);if(r)return Result(r,false); }
 return Result(r,true);
}
}
