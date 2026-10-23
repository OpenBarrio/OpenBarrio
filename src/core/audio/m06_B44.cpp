#include "ts3/audio/m06_B44.h"
#include <cstring>
namespace ts3::audio::b44 {
bool __fastcall Matches(View* self, void*, uint32_t lo, uint32_t hi) {
    const auto* key = reinterpret_cast<const uint32_t*>(reinterpret_cast<const char*>(self)-0x20);
    if (lo == key[0] && hi == key[1]) return true;
    if (!self->parent) return false;
    auto* view = reinterpret_cast<char*>(self->parent)+0x28;
    return (CallGameVirtual<uint32_t,2>(view,lo,hi)&0xff) != 0;
}
static bool Read(View* self, uint32_t key, void* out, uint16_t type, size_t count) {
    Value* value = nullptr;
    if (!(CallGameVirtual<uint32_t,8>(self,key,&value)&0xff)) return false;
    if (value->type != type) return false;
    const void* source = (value->flags & 0x30) ? value->data.pointer : value->data.bytes;
    if (type == 13) {
        float f; std::memcpy(&f,source,4);
#if defined(_M_IX86)
        __asm { fld f }
        __asm { mov eax, out }
        __asm { fstp dword ptr [eax] }
#else
        std::memcpy(out,&f,4);
#endif
    } else std::memcpy(out,source,count);
    return true;
}
bool __fastcall GetByte(View* s, void*, uint32_t k, void* o) { return Read(s,k,o,1,1); }
bool __fastcall GetInt(View* s, void*, uint32_t k, void* o) { return Read(s,k,o,9,4); }
bool __fastcall GetWord(View* s, void*, uint32_t k, void* o) { return Read(s,k,o,10,4); }
bool __fastcall GetFloat(View* s, void*, uint32_t k, void* o) { return Read(s,k,o,13,4); }
bool __fastcall Clear(Owner* s, void*) {
    // Original timeout datum at 0x0103A56C is UINT32_MAX.
    s->mutex.Lock(ts3::thread::kTimeoutNone);
    if (s->resource) {
        CallGameVirtual<void,4>(s->resource);
        if (s->resource) {
            void* p=s->resource; s->resource=nullptr;
            CallGameVirtual<void,2>(p);
        }
    }
    if (s->subscription) {
        void* p=s->subscription; s->subscription=nullptr;
        CallGameVirtual<void,5>(p);
    }
    s->mutex.Unlock();
    return true;
}
void __stdcall Destroy(Entry* first, Entry* last) {
    for (;first<last;++first) {
#if defined(_M_IX86)
        first->value.table=reinterpret_cast<void*>(0x00FB96B4);
#else
        first->value.table=nullptr;
#endif
        if (first->value.flags&4) CallGameMethod<void,0x004F55A0>(&first->value,0);
    }
}
Entry* __cdecl Forward(Entry* first, Entry* last, Entry* out) {
    while (first != last) {
        out->key=first->key;
        CallGameMethod<void*,0x004F62C0>(&out->value,&first->value);
        ++first; ++out;
    }
    return out;
}
Entry* __cdecl Backward(Entry* first, Entry* last, Entry* out) {
    while (last != first) {
        --last; --out;
        out->key=last->key;
        CallGameMethod<void*,0x004F62C0>(&out->value,&last->value);
    }
    return out;
}
Label* __fastcall InitLabel(Label* s, void*) {
#if defined(_M_IX86)
    s->name=reinterpret_cast<const char*>(0x0103A620);
    s->begin=s->end=reinterpret_cast<const char*>(0x011C7E54);
    s->capacity=reinterpret_cast<const char*>(0x011C7E56);
#else
    static const char empty[3] = {};
    s->name="Audio/ResourceConfiguration";
    s->begin=s->end=empty; s->capacity=empty+2;
#endif
    return s;
}
// Verified constant-return twins FileType/MemoryType; only the immediate differs.
uint32_t __fastcall Type(void*, void*) { return 0x00FCCECC; }
Entry* __fastcall Copy(Entry* s, void*, const Entry* source) {
    s->key=source->key;
#if defined(_M_IX86)
    s->value.table=reinterpret_cast<void*>(0x00FB96B4);
#else
    s->value.table=nullptr;
#endif
    s->value.flags=0; s->value.type=0;
    CallGameMethod<void*,0x004F62C0>(&s->value,&source->value);
    return s;
}
}
