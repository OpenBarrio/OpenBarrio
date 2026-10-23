#pragma once
#include "ts3/anim/m05_layouts.h"
#include "ts3/services/cls_4a4ba0_Services.h"
namespace ts3::anim::b23 {
using Link = services::ServiceNodeBase;
struct Key { uint32_t word[4]; };
struct Value { Key key; void* binding; uint32_t extra; };
struct Node { Link links; Value value; };
struct Tree { uint32_t comparator; Link anchor; uint32_t count; void* allocator; };
struct Result { Link* node; uint8_t inserted; };
struct Samples { uint8_t opaque00[16]; float interval; uint16_t frames; uint8_t opaque16[18]; int32_t nameOffset; };
struct View { uint8_t opaque00[12]; Samples* samples; uint8_t opaque10[12]; void* name; };
struct Resource { ResourcePrefix base; uint8_t loaded; uint8_t pad[3]; Tree bindings; void* ik; View* view; void* raw; void* eventBegin; void* eventEnd; void* eventCapacity; void* eventDescriptor; };
TS3_X86_SIZE(Node,40); TS3_X86_SIZE(Tree,28); TS3_X86_OFFSET(Resource,bindings,0x2c); TS3_X86_OFFSET(Resource,view,0x4c); TS3_X86_OFFSET(Samples,nameOffset,0x28); TS3_X86_OFFSET(View,name,0x1c);
// TS3W.exe 0x0057A500: queries the clip resource's supported interface IDs.
void* __fastcall Query(Resource*,void*,uint32_t);
// TS3W.exe 0x0057A690: returns the validated view's name or null.
void* __fastcall Name(Resource*,void*);
// TS3W.exe 0x0057A6A0: returns the sampled clip payload or null.
Samples* __fastcall Data(Resource*,void*);
// TS3W.exe 0x0057A740: computes (unsigned frame count minus one) times interval.
double __fastcall Duration(Resource*,void*);
// TS3W.exe 0x0057A7A0: returns a relative clip name or the original empty string.
const char* __fastcall ClipName(Resource*,void*);
// TS3W.exe 0x0057A870: allocates and links a copied binding node; keeps the node+16 null check.
Link** __fastcall InsertAt(Tree*,void*,Link**,Link*,const Value*,uint8_t);
// TS3W.exe 0x0057A940: destroys a resource base and optionally frees it, returning this.
void* __fastcall DeleteBase(void*,void*,uint32_t);
// TS3W.exe 0x0057A970: releases binding resources, nodes, raw bytes and IK in original order.
uintptr_t __fastcall Clear(Resource*,void*);
// TS3W.exe 0x0057AA10: clears the clip and sets only the low return byte to one.
uintptr_t __fastcall Unload(Resource*,void*);
// TS3W.exe 0x0057AA20: inserts a unique binding key, preserving result padding.
Result* __fastcall Insert(Tree*,void*,Result*,const Value*,uintptr_t);
// TS3W.exe 0x0057AB90: returns the clip resource type ID.
uint32_t __fastcall Type(Resource*,void*);
// TS3W.exe 0x0057ABA0: inserts with a hint, accepting both rightmost and sentinel hints.
Link** __fastcall InsertHint(Tree*,void*,Link**,Link*,const Value*,uintptr_t);
// TS3W.exe 0x0057AD50: finds or inserts a key and returns its binding address; copies entry ESP-4 into the temporary tail.
void** __fastcall Binding(Tree*,void*,const Key*);
}
