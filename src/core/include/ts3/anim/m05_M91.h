#pragma once
#include "ts3/anim/m05_B25.h"
namespace ts3::anim::m91 {
using b25::Event;
struct TextEvent { Event base; char text[128]; };
struct KeyEvent { Event base; uint32_t value, untouched, keyLow, keyHigh, tagLow, tagHigh; };
struct WordEvent { Event base; uint32_t value; };
struct NamedEvent { Event base; const char* name; uint32_t untouched, fields[5]; };
#if defined(_M_IX86)
static_assert(offsetof(NamedEvent,fields)==0x30 && offsetof(KeyEvent,tagLow)==0x38);
#endif
// TS3W.exe 0x0057FC60: Read bounded text for version >= 0x103; consume 128 bytes but subtract 32 from count.
uint8_t __fastcall ReadText(TextEvent*,void*,uint32_t,uint8_t**,uint32_t*,uint32_t);
// TS3W.exe 0x0057FCB0: Compare base and inline text, preserving the defined upper EAX bits.
uint32_t __fastcall EqualText(TextEvent*,void*,const TextEvent*);
// TS3W.exe 0x0057FD10: Read four words with optional in-place endian swap; ignore word one and subtract four from byte count.
uint8_t __fastcall ReadKey(KeyEvent*,void*,uint32_t,uint8_t**,uint32_t*,uint32_t);
// TS3W.exe 0x0057FD70: Compare base and key fields in EA's register-observable order.
uint32_t __fastcall EqualKey(KeyEvent*,void*,const KeyEvent*);
// TS3W.exe 0x0057FDC0: Compare base and payload word, preserving the upper payload bits in EAX.
uint32_t __fastcall EqualWord(WordEvent*,void*,const WordEvent*);
// TS3W.exe 0x0057FDE0: Compare base, five words and nullable text, retaining upper EAX on failure.
uint32_t __fastcall EqualNamed(NamedEvent*,void*,const NamedEvent*);
}
