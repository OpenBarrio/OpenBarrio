#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m122 {
struct String { void* begin; void* end; void* capacity; void* descriptor; };
struct Style {
    void* resources[20];
    int32_t extra, first, second, third, category;
    unsigned char flags[6], padding[2];
    String sound;
};
struct Range { Style* begin; Style* end; };
struct Manager { unsigned char opaque[0x4c]; };
struct InlineVector {
    uint32_t* begin;
    uint32_t* end;
    uint32_t* capacity;
    void* descriptor;
    uint32_t* storageBegin;
    uint32_t storage[42];
};
struct Node { Node* next; Node* previous; InlineVector value; };
struct List { Node* next; Node* previous; void* descriptor; };
TS3_X86_SIZE(Style,0x7c);
TS3_X86_OFFSET(Style,first,0x54);
TS3_X86_SIZE(Node,0xc4);
// TS3W.exe 0x00844980: Select the closest style, optionally applying a fallback first.
bool __fastcall Select(Manager*,void*,uint32_t,int32_t,int32_t,int32_t,int32_t,Style*,bool);
// TS3W.exe 0x00844BE0: Select a style with mapped rather than raw first/second values.
bool __fastcall SelectMapped(Manager*,void*,uint32_t,int32_t,int32_t,int32_t,int32_t,Style*);
// TS3W.exe 0x00844EC0: Select a mapped style in category minus one.
bool __fastcall SelectDefault(Manager*,void*,uint32_t,int32_t,int32_t,int32_t,Style*);
// TS3W.exe 0x00844EF0: Allocate and append an inline-vector list node, returning the previous tail.
Node* __fastcall Append(List*,void*,const void*);
}
