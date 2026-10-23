#include "ts3/audio/m06_B46.h"
#include <cstring>
namespace ts3::audio::b46 {
using ts3::CallGameFunction;
using ts3::CallGameMethod;
using ts3::CallGameVirtual;
namespace {
void InitVariant(Variant* v) {
#if defined(_M_IX86)
    v->table=reinterpret_cast<void**>(0x00FB96B4);
#else
    v->table=nullptr; // Native storage; game variant operations still require an external implementation.
#endif
    v->flags=0; v->type=0;
}
Entry* Find(PropertySet* s,uint32_t key) {
    return CallGameFunction<Entry*,0x008A7900>(s->begin,s->end,&key,uint32_t(s->sorted));
}
uint8_t Lookup(PropertySet* s,uint32_t key,void** out,bool mutableValue) {
    if (!s->localOnly) {
        if (key==0x25df0108 && s->groups.type) { *out=&s->groups; return 1; }
        if (key==0x7cb81a35 && s->map.type) { *out=&s->map; return 1; }
    }
    Entry* e=Find(s,key);
    if (e!=s->end && key>=e->key) { *out=&e->value; return 1; }
    if (s->localOnly || !s->parent) return 0;
    if (mutableValue) return CallGameVirtual<uint8_t,8>(&s->parent->properties,key,out);
    return CallGameVirtual<uint8_t,7>(&s->parent->properties,key,out);
}
struct StringDefaults { const void *begin,*end,*capacity; const char* label; };
StringVector* InitStrings(StringVector* s,uint32_t count,void* allocator,bool second) {
    CallGameMethod<void,0x00ABBA90>(s,count,allocator);
    StringDefaults defaults{};
#if defined(_M_IX86)
    defaults.begin=reinterpret_cast<void*>(0x011C7E54);
    defaults.end=defaults.begin;
    defaults.capacity=reinterpret_cast<void*>(second?0x011C7E56:0x011C7E55);
    defaults.label=reinterpret_cast<const char*>(0x0103A620);
#endif
    if (second) CallGameFunction<void,0x00412BF0>(s->begin,count,&defaults,allocator);
    else CallGameFunction<void,0x004F4550>(s->begin,count,&defaults,allocator);
    s->end=reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(s->begin)+16*count);
    return s;
}
}
uint32_t* Difference(uint32_t* a,uint32_t* ae,uint32_t* b,uint32_t* be,uint32_t* out) {
    while (a!=ae && b!=be) {
        if (*b>*a) *out++=*a++;
        else { if (*b==*a) ++a; ++b; }
    }
    const auto n=ae-a;
    std::memmove(out,a,n*sizeof(uint32_t));
    return out+n;
}
uint32_t* Union(uint32_t* a,uint32_t* ae,uint32_t* b,uint32_t* be,uint32_t* out) {
    while (a!=ae && b!=be) {
        if (*b>*a) *out++=*a++;
        else if (*b<*a) *out++=*b++;
        else { *out++=*a++; ++b; }
    }
    auto n=ae-a;
    std::memmove(out,a,n*sizeof(uint32_t)); out+=n;
    n=be-b;
    std::memmove(out,b,n*sizeof(uint32_t));
    return out+n;
}
uint8_t PropertySet::Get(uint32_t key,void** out) { return Lookup(this,key,out,false); }
uint8_t PropertySet::GetMutable(uint32_t key,void** out) { return Lookup(this,key,out,true); }
uint8_t PropertySet::Contains(uint32_t key) {
    Entry* e=Find(this,key);
    if (e!=end && key>=e->key) return 1;
    if (parent) return CallGameVirtual<uint8_t,6>(&parent->properties,key);
    return 0;
}
uintptr_t PropertySet::Enumerate(void* target,uint32_t clear) {
    struct KeySet { void* table; void *next,*previous,*root; uint8_t flag; uint32_t size; };
    auto set=static_cast<KeySet*>(target);
    if (uint8_t(clear)) {
        CallGameMethod<void,0x006A0850>(target,set->root);
        set->next=&set->next; set->previous=&set->next; set->root=nullptr; set->flag=0; set->size=0;
    }
    Entry* stop=end;
    for (Entry* e=begin;e!=stop;++e) {
        uint32_t key=e->key; uintptr_t result[2];
        clear &= ~uint32_t(255);
        CallGameMethod<void,0x006A1BE0>(target,result,&key,clear);
    }
    if (!localOnly) {
        if (parent) return CallGameVirtual<uintptr_t,3>(&parent->properties,target,uint32_t(0));
        return 0;
    }
    return reinterpret_cast<uintptr_t>(this);
}
StringVector* StringVector::InitA(uint32_t n,void* a) { return InitStrings(this,n,a,false); }
StringVector* StringVector::InitB(uint32_t n,void* a) { return InitStrings(this,n,a,true); }
uintptr_t EntryVector::InsertSlow(Entry* at,Entry* source) {
    if (end!=capacity) {
        Entry* adjusted=source;
        if (reinterpret_cast<uintptr_t>(source)>=reinterpret_cast<uintptr_t>(at) &&
            reinterpret_cast<uintptr_t>(source)<reinterpret_cast<uintptr_t>(end)) adjusted=source+1;
        if (end) {
            end->key=(end-1)->key;
            InitVariant(&end->value);
            CallGameMethod<uintptr_t,0x004F62C0>(&end->value,&(end-1)->value);
        }
        CallGameFunction<void,0x008A7830>(at,end-1,end);
        at->key=adjusted->key;
        auto result=CallGameMethod<uintptr_t,0x004F62C0>(&at->value,&adjusted->value);
        ++end;
        return result;
    }
    const auto count=(reinterpret_cast<intptr_t>(end)-reinterpret_cast<intptr_t>(begin))/sizeof(Entry);
    const uint32_t newCapacity=count?uint32_t(count)*2:1;
    Entry* storage=nullptr;
    if (newCapacity) storage=CallGameFunction<Entry*,0x004E3F90>(uint32_t(sizeof(Entry))*newCapacity,label,0,0,
        "..\\..\\..\\EASTL\\include\\EASTL\\vector.h",uint32_t(231));
    Entry* cursor;
    CallGameFunction<void,0x008A7950>(&cursor,begin,at,storage,label);
    Entry* inserted=cursor;
    if (inserted) {
        inserted->key=source->key; InitVariant(&inserted->value);
        CallGameMethod<void,0x004F62C0>(&inserted->value,&source->value);
    }
    CallGameFunction<void,0x008A7950>(&cursor,at,end,inserted+1,label);
    CallGameMethod<void,0x008A77B0>(this,begin,end);
    if (begin) CallGameFunction<void,0x004E3F70>(begin);
    end=cursor; begin=storage; capacity=storage+newCapacity;
    return newCapacity;
}
Entry* EntryVector::Insert(Entry* at,Entry* source) {
    const auto index=(reinterpret_cast<intptr_t>(at)-reinterpret_cast<intptr_t>(begin))/sizeof(Entry);
    if (end!=capacity && at==end) {
        Entry* slot=end++;
        if (slot) CallGameMethod<void,0x008A78D0>(slot,source);
    } else InsertSlow(at,source);
    return begin+index;
}
uint8_t Loader::Create(void* input,void** output,uint32_t a2,uint32_t a3) {
    CallGameMethod<void,0x004E16F0>(mutex,reinterpret_cast<const void*>(0x0103A56C));
    auto storage=CallGameFunction<void*,0x004E3F90>(uint32_t(176),"Audio/ResourceConfiguration",0,0,0,0);
    void* resource=storage?CallGameMethod<void*,0x008A7BF0>(storage):nullptr;
    if (resource) {
        CallGameVirtual<void,0>(resource);
        auto key=CallGameVirtual<const uint32_t*,4>(input);
        struct ResourcePrefix { void** table; uint32_t refs; uint32_t key[4]; };
        std::memcpy(static_cast<ResourcePrefix*>(resource)->key,key,16);
        if (CallGameVirtual<uint8_t,9>(this,input,resource,a2,a3)) {
            *output=resource;
            CallGameVirtual<void,0>(resource);
            CallGameVirtual<void,1>(resource);
            CallGameMethod<void,0x004E17B0>(mutex);
            return 1;
        }
    }
    CallGameVirtual<void,8>(input);
    if (resource) CallGameVirtual<void,1>(resource);
    CallGameMethod<void,0x004E17B0>(mutex);
    return 0;
}
uint8_t Loader::ReadProperties(void* input) {
    int32_t count;
    if (!CallGameFunction<uint8_t,0x004DD700>(input,&count,0)) return 0;
    bool good=true;
    Variant value; InitVariant(&value);
    uint32_t key; // EA still calls the setter after failure, using the previous key/value.
    for (int32_t i=0;i<count;++i) {
        good=good && CallGameFunction<uint8_t,0x004DD700>(input,&key,0) &&
            CallGameMethod<uint8_t,0x008A82B0>(this,input,&value);
        CallGameVirtual<void,4>(&properties,key,&value);
    }
#if defined(_M_IX86)
    value.table=reinterpret_cast<void**>(0x00FB96B4);
#endif
    if (value.flags&4) CallGameMethod<void,0x004F55A0>(&value,0);
    return good;
}
}
