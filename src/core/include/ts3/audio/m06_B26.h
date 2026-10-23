#pragma once
#include "ts3/layout.h"
namespace ts3::audio::b26impl {
struct Node { Node* left; Node* right; Node* parent; uint8_t color; uint8_t pad[3]; uint32_t key; void* object; uint8_t flag; uint8_t tail[3]; };
struct Header { Node* left; Node* right; Node* parent; uint8_t color; uint8_t pad[3]; };
struct Value { uint32_t key; void* object; uint8_t flag; };
struct Result { Node* node; uint8_t inserted; };
struct Tree { uint32_t label; Header header; uint32_t count; Node* free; uint8_t storage[120]; };
struct FlaggedTree { Tree tree; uint8_t extra[104]; };
struct Curve { void* begin; void* end; void* capacity; uint32_t label; void* storage; };
struct Sound { void* primary; void* secondary; void* third; uint8_t base[0x18a0-12]; Curve* curves[4]; float parameters[4]; uint8_t flags[4]; char16_t* begin; char16_t* end; char16_t* capacity; uint32_t label; };
struct BucketNode { uint32_t key; uint32_t hash; Tree tree; uint8_t padding[0xb8-8-0x94]; BucketNode* next; };
// Bucket nodes use the 120-byte pool, rather than Tree's largest inline pool.
struct BucketIterator { BucketNode* node; BucketNode** bucket; };
struct Hash { void* buckets; uint32_t bucketCount; void* sentinel; uint32_t count; };
TS3_X86_OFFSET(Node,key,0x10); TS3_X86_OFFSET(Node,object,0x14);
TS3_X86_OFFSET(Tree,header,4); TS3_X86_OFFSET(Tree,count,0x14); TS3_X86_OFFSET(Tree,free,0x18);
TS3_X86_OFFSET(Sound,curves,0x18a0); TS3_X86_OFFSET(Sound,begin,0x18c4);
// TS3W.exe 0x00871690: Insert a retained flagged value from the pool and rebalance; no allocation-failure guard.
Node** InsertFlagged(Tree*,Node**,Node*,const Value*,uint32_t);
// TS3W.exe 0x00871720: Clone one retained tree node, preserving its color.
Node* CloneNode(Tree*,const Node*,Node*);
// TS3W.exe 0x00871780: Initialize the 24-byte-node inline pooled tree.
Tree* Construct(Tree*);
// TS3W.exe 0x00871800: Unique insert into the unflagged retained tree.
Result* Unique(Tree*,Result*,const Value*,uint32_t);
// TS3W.exe 0x00871890: Unique insert into the flagged retained tree.
Result* UniqueFlagged(Tree*,Result*,const Value*,uint32_t);
// TS3W.exe 0x00871920: Clone a retained subtree, left recursively and right iteratively.
Node* CloneTree(Tree*,const Node*,Node*);
// TS3W.exe 0x008719D0: Destroy curves and filename, then tail-call the base sound destructor.
uintptr_t DestroySound(Sound*);
// TS3W.exe 0x00871AB0: Initialize the 28-byte-node inline pooled tree.
Tree* ConstructFlagged(Tree*);
// TS3W.exe 0x00871B30: Unique-insert wrapper returning the output pointer.
Result* Insert(Tree*,Result*,const Value*);
// TS3W.exe 0x00871B60: Flagged unique-insert wrapper returning the output pointer.
Result* InsertWithFlag(Tree*,Result*,const Value*);
// TS3W.exe 0x00871B90: Assign a retained tree, with self-assignment a no-op.
Tree* Assign(Tree*,const Tree*);
// TS3W.exe 0x00871C20: Destroy sound and optionally free it; returns the original pointer.
Sound* DeleteSound(Sound*,uint32_t);
// TS3W.exe 0x00871C40: Copy-construct the unflagged pooled tree.
Tree* CopyConstruct(Tree*,const Tree*);
// TS3W.exe 0x00871CD0: Unlink a hash node, release its retained tree, free it and advance the iterator.
BucketIterator* Erase(Hash*,BucketIterator*,BucketNode*,BucketNode**);
}

// Thiscall ABI facade for registration's generated member adapters. It carries no
// native state: each entry explicitly views the original object as its typed layout.
namespace ts3::audio {
#if defined(_M_IX86)
struct b26 {
 uint32_t InsertFlagged(void* out,void* parent,void* value,uint32_t force) {return uint32_t(b26impl::InsertFlagged(reinterpret_cast<b26impl::Tree*>(this),static_cast<b26impl::Node**>(out),static_cast<b26impl::Node*>(parent),static_cast<const b26impl::Value*>(value),force));}
 uint32_t CloneNode(void* source,void* parent) {return uint32_t(b26impl::CloneNode(reinterpret_cast<b26impl::Tree*>(this),static_cast<const b26impl::Node*>(source),static_cast<b26impl::Node*>(parent)));}
 uint32_t CloneTree(void* source,void* parent) {return uint32_t(b26impl::CloneTree(reinterpret_cast<b26impl::Tree*>(this),static_cast<const b26impl::Node*>(source),static_cast<b26impl::Node*>(parent)));}
 uint32_t Construct() {return uint32_t(b26impl::Construct(reinterpret_cast<b26impl::Tree*>(this)));}
 uint32_t Unique(void* out,void* value,uint32_t flag) {return uint32_t(b26impl::Unique(reinterpret_cast<b26impl::Tree*>(this),static_cast<b26impl::Result*>(out),static_cast<const b26impl::Value*>(value),flag));}
 uint32_t UniqueFlagged(void* out,void* value,uint32_t flag) {return uint32_t(b26impl::UniqueFlagged(reinterpret_cast<b26impl::Tree*>(this),static_cast<b26impl::Result*>(out),static_cast<const b26impl::Value*>(value),flag));}
 uint32_t DestroySound() {return b26impl::DestroySound(reinterpret_cast<b26impl::Sound*>(this));}
 uint32_t DeleteSound(uint32_t flags) {return uint32_t(b26impl::DeleteSound(reinterpret_cast<b26impl::Sound*>(this),flags));}
 uint32_t ConstructFlagged() {return uint32_t(b26impl::ConstructFlagged(reinterpret_cast<b26impl::Tree*>(this)));}
 uint32_t Insert(void* out,void* value) {return uint32_t(b26impl::Insert(reinterpret_cast<b26impl::Tree*>(this),static_cast<b26impl::Result*>(out),static_cast<const b26impl::Value*>(value)));}
 uint32_t InsertWithFlag(void* out,void* value) {return uint32_t(b26impl::InsertWithFlag(reinterpret_cast<b26impl::Tree*>(this),static_cast<b26impl::Result*>(out),static_cast<const b26impl::Value*>(value)));}
 uint32_t Assign(void* source) {return uint32_t(b26impl::Assign(reinterpret_cast<b26impl::Tree*>(this),static_cast<const b26impl::Tree*>(source)));}
 uint32_t CopyConstruct(void* source) {return uint32_t(b26impl::CopyConstruct(reinterpret_cast<b26impl::Tree*>(this),static_cast<const b26impl::Tree*>(source)));}
 uint32_t Erase(void* out,void* node,void* bucket) {return uint32_t(b26impl::Erase(reinterpret_cast<b26impl::Hash*>(this),static_cast<b26impl::BucketIterator*>(out),static_cast<b26impl::BucketNode*>(node),static_cast<b26impl::BucketNode**>(bucket)));}
};
#endif
}
