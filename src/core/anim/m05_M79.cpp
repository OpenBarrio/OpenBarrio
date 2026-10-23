#include "ts3/anim/m05_M79.h"
namespace ts3::anim {
B12Record* Backward7DC0(const B12Record* first, const B12Record* last, B12Record* dest) {
    while (last != first) {
        --last;
        --dest;
        Assign8D30(dest,nullptr,last);
    }
    return dest;
}
void Construct7E00(B12Record* dest, uint32_t count, const B12Record* value) {
    while (count > 0) {
        if (dest) Copy8C50(dest,nullptr,value);
        dest=reinterpret_cast<B12Record*>(reinterpret_cast<uintptr_t>(dest)+sizeof(B12Record));
        --count;
    }
}
static void InitM79String(B12String& s) {
    s.begin=s.end=s.storage;
    s.capacity=s.storage+96;
    s.storage[0]=0;
}
B12Record* __fastcall Init7E30(B12Record* s, void*) {
    InitM79String(s->first);
    InitM79String(s->second);
    for (unsigned i=0;i<6;++i) s->words[i]=0;
    for (unsigned i=6;i<9;++i) s->words[i]=0xffffffff;
    return s;
}
uint8_t __fastcall Resolve7EA0(B12Record* s, void*, void*) {
    if (!(s->words[0] | s->words[1])) return 0;
    auto object=CallGameMethod<void*,0x00935830>(&s->words[0]);
    if (!object) return 0;
    void* type=nullptr;
#if defined(_M_IX86)
    type=*reinterpret_cast<void**>(0x011dc484);
#endif
    auto resource=CallGameMethod<unsigned char*,0x00935770>(object,type);
    if (!resource) return 0;
    auto rig=CallGameVirtual<unsigned char*,8>(resource+8);
    if (!rig) return 0;
    if (!s->words[10]) return 1;
    auto skeleton=rig+0x28;
    auto joint=CallGameMethod<int32_t,0x00575b00>(skeleton,s->words[10]);
    if (joint == -1) return 0;
    auto parent=CallGameMethod<int32_t,0x005756d0>(skeleton,joint);
    if (parent == -1) return 0;
    s->words[7]=parent;
    auto name=CallGameMethod<const char*,0x00575670>(skeleton,parent);
    CallGameMethod<void,0x005b7980>(&s->second,name);
    s->words[10]=CallGameMethod<uint32_t,0x00575690>(skeleton,s->words[7]);
    return 1;
}
void Fill7F50(B12Record* first, B12Record* last, const B12Record* value) {
    while (first != last) {
        Assign8D30(first,nullptr,value);
        ++first;
    }
}
static void ClearM79String(B12String& s) {
    if (s.begin != s.end) {
        *s.begin=0;
        s.end=s.begin;
    }
}
uint8_t __fastcall Reset7F80(ClipIkInfo* s, void*) {
    auto end=static_cast<M79Slot*>(s->slotEnd);
    for (auto slot=static_cast<M79Slot*>(s->slotBegin);slot != end;++slot) {
        slot->index=-1;
        for (auto p=slot->begin;p != slot->end;++p) {
            for (unsigned i=6;i<9;++i) p->words[i]=0xffffffff;
            for (unsigned i=0;i<6;++i) p->words[i]=0;
            ClearM79String(p->first);
            ClearM79String(p->second);
        }
    }
    s->namespaceEnd=s->namespaceBegin;
    s->hashEnd=s->hashBegin;
    s->rotation={0,0,0,1};
    return 1;
}
}
