#include "ts3/audio/m06_B25.h"
#include <cstring>
#include <intrin.h>
namespace ts3::audio::b25 {
namespace {
Node* Sentinel(Tree* t) { return reinterpret_cast<Node*>(&t->first); }
void Lock(Queue* q) {
#if defined(_M_IX86)
    using F=void(__stdcall*)(void*); (*reinterpret_cast<F*>(0xF9512C))(q->mutex);
#else
    (void)q; AbortNotRewritten("B25 queue lock");
#endif
}
void Unlock(Queue* q) {
#if defined(_M_IX86)
    using F=void(__stdcall*)(void*); (*reinterpret_cast<F*>(0xF95130))(q->mutex);
#else
    (void)q; AbortNotRewritten("B25 queue unlock");
#endif
}
void* Allocator() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x11CB864);
#else
    AbortNotRewritten("B25 payload allocator"); return nullptr;
#endif
}
}
const void* SoundName(void*, uint32_t) {
#if defined(_M_IX86)
    return reinterpret_cast<const void*>(0xF97F7A);
#else
    static const char16_t name[]=u""; return name;
#endif
}
uint8_t GetFlag(void* self) { return static_cast<uint8_t*>(self)[0x18c1]; }
uint8_t SetFlag(void* self, uint32_t value) { return static_cast<uint8_t*>(self)[0x18c1]=static_cast<uint8_t>(value); }
void* DeleteInterface(void* self, uint32_t flags) {
    return CallGameMethod<void*,0x871C20>(static_cast<uint8_t*>(self)-8,flags);
}
Payload* Construct(Payload* self, uint32_t tag, const void* data, uint32_t size) {
#if defined(_M_IX86)
    self->vtable=reinterpret_cast<void*>(0x107396C);
#else
    self->vtable=nullptr;
#endif
    _InterlockedExchange(reinterpret_cast<volatile long*>(&self->refs),0);
#if defined(_M_IX86)
    self->vtable=reinterpret_cast<void*>(0x102A210);
#endif
    self->tag=tag; self->size=size;
    if (data) {
        self->data=CallGameMethod<void*,0x4E69D0>(Allocator(),size,0);
        if(self->data) { std::memcpy(self->data,data,size); return self; }
    }
    self->size=0; self->tag=0; self->data=nullptr;
    return self;
}
Payload* Destroy(Payload* self, uint32_t flags) {
#if defined(_M_IX86)
    self->vtable=reinterpret_cast<void*>(0x102A210);
#endif
    if(self->data && self->size) CallGameMethod<void,0x4E5920>(Allocator(),self->data);
    self->size=0; self->tag=0; self->data=nullptr;
#if defined(_M_IX86)
    self->vtable=reinterpret_cast<void*>(0x107396C);
#endif
    if(flags&1) CallGameFunction<void,0x4E3F70>(self);
    return self;
}
Entry* CopyEntry(Entry* out, const uint32_t* key, const RefValue* value) {
    out->key=*key; out->object=value->object;
    if(out->object) CallGameVirtual<void,0>(out->object);
    out->flag=value->flag;
    return out;
}
uint8_t Contains(Queue* self, uint32_t key) {
    Lock(self);
    Group** bucket=self->buckets;
    while(!*bucket) ++bucket;
    Group* group=*bucket;
    Group* end=self->buckets[self->count];
    while(group!=end) {
        Node* sentinel=Sentinel(&group->tree);
        Node* candidate=sentinel;
        for(Node* n=group->tree.root;n;) {
            if(n->key>=key) { candidate=n; n=n->left; } else n=n->right;
        }
        if(candidate!=sentinel && key>=candidate->key) { Unlock(self); return 1; }
        group=group->next;
        if(!group) { do { ++bucket; } while(!*bucket); group=*bucket; }
    }
    Unlock(self); return 0;
}
void Update(Queue* self, float value) {
    Lock(self);
    if(self->active) {
        self->update=value;
        Group** bucket=self->buckets;
        while(!*bucket) ++bucket;
        Group* group=*bucket;
        Group* end=self->buckets[self->count];
        while(group!=end) {
            Node* sentinel=Sentinel(&group->tree);
            for(Node* n=group->tree.last;n!=sentinel;n=CallGameFunction<Node*,0x4E0B20>(n)) {
                void* sound=n->object;
                if(sound) {
                    CallGameVirtual<void,0>(sound);
                    if(static_cast<uint8_t*>(sound)[0xadc]&1) {
                        // EA uses FLD/FSTP for the argument, quieting signaling NaNs,
                        // but leaves the MOVSS-stored update field's bits unchanged.
                        float argument=self->update;
#if defined(_M_IX86)
                        __asm { fld argument }
                        __asm { fstp argument }
#endif
                        CallGameVirtual<void,3>(static_cast<uint8_t*>(sound)+sizeof(void*),uint32_t(0x25df0108),argument,1);
                    }
                    CallGameVirtual<void,1>(sound);
                }
            }
            group=group->next;
            if(!group) { do { ++bucket; } while(!*bucket); group=*bucket; }
        }
    }
    Unlock(self);
}
Iterator* Find(Table* self, Iterator* out, const uint32_t* key) {
    Group** bucket=self->buckets+key[0]%self->count;
    for(Group* n=*bucket;n;n=n->next) {
        if(n->key[0]==key[0] && n->key[1]==key[1]) { out->node=n; out->bucket=bucket; return out; }
    }
    out->bucket=self->buckets+self->count; out->node=*out->bucket; return out;
}
uintptr_t SetProperty(void* self, uint32_t key, uintptr_t value, uint32_t extra) {
    auto* bytes=static_cast<uint8_t*>(self);
    if(key==0x2081b4e6) { uint32_t v=static_cast<uint32_t>(value); std::memcpy(bytes+0x18ac,&v,4); return key; }
    if(key==0x9fa9372e) {
        auto* begin=reinterpret_cast<const char16_t*>(value); auto* end=begin;
        while(*end) ++end;
        return CallGameMethod<uintptr_t,0x408E20>(bytes+0x18c4,begin,end);
    }
    return CallGameMethod<uintptr_t,0x8ADBE0>(self,key,value,extra);
}
int32_t Clear(Tree* self) {
    Recycle(self,self->root);
    self->last=Sentinel(self); self->first=Sentinel(self); self->root=nullptr; self->flag=0; self->count=0;
    return 0;
}
void Recycle(Tree* self, Node* node) {
    while(node) {
        Recycle(self,node->right);
        Node* next=node->left;
        if(node->object) CallGameVirtual<void,1>(node->object);
        node->right=self->free; self->free=node; node=next;
    }
}
Node** Insert(Tree* self, Node** out, Node* parent, const Entry* value, uint8_t forceLeft) {
    uint32_t direction=(!forceLeft && parent!=Sentinel(self) && value->key>=parent->key)?1:0;
    Node* node=self->free;
    if(node) self->free=node->right;
    // Deliberately no empty-pool guard: EA computes null + 16 and writes through it.
    node->key=value->key; node->object=value->object;
    if(node->object) CallGameVirtual<void,0>(node->object);
    CallGameFunction<void,0x4E0C40>(node,parent,Sentinel(self),direction);
    ++self->count; *out=node; return out;
}
#if defined(_M_IX86)
const void* __fastcall HookSoundName(void* s, void*, uint32_t a) { return SoundName(s,a); }
uint8_t __fastcall HookGetFlag(void* s, void*) { return GetFlag(s); }
uint8_t __fastcall HookSetFlag(void* s, void*, uint32_t a) { return SetFlag(s,a); }
void* __fastcall HookDeleteInterface(void* s, void*, uint32_t a) { return DeleteInterface(s,a); }
Payload* __fastcall HookConstruct(Payload* s, void*, uint32_t a, const void* b, uint32_t c) { return Construct(s,a,b,c); }
Payload* __fastcall HookDestroy(Payload* s, void*, uint32_t a) { return Destroy(s,a); }
uint8_t __fastcall HookContains(Queue* s, void*, uint32_t a) { return Contains(s,a); }
void __fastcall HookUpdate(Queue* s, void*, float a) { return Update(s,a); }
Iterator* __fastcall HookFind(Table* s, void*, Iterator* a, const uint32_t* b) { return Find(s,a,b); }
uintptr_t __fastcall HookSetProperty(void* s, void*, uint32_t a, uintptr_t b, uint32_t c) { return SetProperty(s,a,b,c); }
int32_t __fastcall HookClear(Tree* s, void*) { return Clear(s); }
void __fastcall HookRecycle(Tree* s, void*, Node* a) { return Recycle(s,a); }
Node** __fastcall HookInsert(Tree* s, void*, Node** a, Node* b, const Entry* c, uint8_t d) { return Insert(s,a,b,c,d); }
#endif
}
