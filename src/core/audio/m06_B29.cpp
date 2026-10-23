#include "ts3/audio/m06_B29.h"
#include <cstring>
#include <utility>
namespace ts3::audio {
namespace {
uint8_t* Lookup(B29Records* self, uint32_t tag, uint32_t* type, uint32_t* size)
{
#if defined(_M_IX86)
    return CallGameMethod<uint8_t*,0x0089F7D0>(self,tag,type,size);
#else
    // Native counterpart of the external packed-record lookup interface.
    auto* p=self->records;
    for(uint32_t i=0;i<self->count;++i) {
        uint32_t h[3]; std::memcpy(h,p,12); p+=12;
        if(h[0]==tag) { if(type) *type=h[2]; if(size) *size=h[1]; return p; }
        p+=(h[1]+3u)&~3u;
    }
    return nullptr;
#endif
}
void CopyFloats(float* out, const uint8_t* p, size_t n)
{
    // fld/fstp affects just word zero. Snapshot the SSE words before any stores
    // so even overlapping output matches the original load-before-store order.
    uint32_t words[9]; std::memcpy(words,p,n*4);
    if((words[0]&0x7f800000u)==0x7f800000u && (words[0]&0x007fffffu))
        words[0]|=0x00400000u;
    std::memcpy(out,words,n*4);
}
}
bool B29Records::Contains(uint32_t tag, uint32_t* type, uint32_t* size)
{
    return Lookup(this,tag,type,size)!=nullptr;
}
bool B29Records::ReadFloat(uint32_t tag, float* out)
{
    uint32_t type=tag,size=0;
    auto* p=Lookup(this,tag,&type,&size);
    if(!p || type!=0) return false;
    CopyFloats(out,p,1);
    return true;
}
bool B29Records::ReadKey(uint32_t tag, uint32_t* out)
{
    uint32_t type=tag,size=0;
    auto* p=Lookup(this,tag,&type,&size);
    if(!p || type!=2) return false;
    // EA stores the first word before loading the second (overlap matters).
    std::memcpy(out,p,4);
    std::memcpy(out+1,p+4,4);
    return true;
}
void* B29Records::ReadString(uint32_t tag)
{
    uint32_t type=tag,size=0;
    auto* p=Lookup(this,tag,&type,&size);
    return p && type==3 ? p : nullptr;
}
bool B29Records::ReadWord(uint32_t tag, uint32_t* out)
{
    // Adapted from verified RendererRecordReader::ReadScalar (0x0089F940):
    // only the type discriminator changes, from one to four.
    uint32_t type=tag,size=0;
    auto* p=Lookup(this,tag,&type,&size);
    if(!p || type!=4) return false;
    std::memcpy(out,p,4);
    return true;
}
void* B29Records::ReadBlob(uint32_t tag, uint32_t* size)
{
    uint32_t type=tag,bytes=0;
    auto* p=Lookup(this,tag,&type,&bytes);
    if(!p || type!=5) return nullptr;
    if(size) *size=bytes;
    return p;
}
bool B29Records::ReadVector(uint32_t tag, float* out)
{
    uint32_t type=tag,size=0;
    auto* p=Lookup(this,tag,&type,&size);
    if(!p || type!=6) return false;
    CopyFloats(out,p,3);
    return true;
}
bool B29Records::ReadMatrix(uint32_t tag, float* out)
{
    uint32_t type=tag,size=0;
    auto* p=Lookup(this,tag,&type,&size);
    if(!p || type!=7) return false;
    CopyFloats(out,p,9);
    return true;
}
bool B29Records::WriteKey(uint32_t tag, uint32_t low, uint32_t high)
{
    uint32_t type=tag,size=0;
    auto* p=Lookup(this,tag,&type,&size);
    if(!p || type!=2) return false;
    std::memcpy(p,&low,4); std::memcpy(p+4,&high,4);
    return true;
}
uint8_t* B29Records::Erase(uint32_t tag)
{
    auto* p=records;
    for(uint32_t i=0;i<count;++i) {
        uint32_t h[2]; std::memcpy(h,p,8); p+=12;
        if(h[0]==tag) {
            if(p) { uint32_t dead=0xffffffffu; std::memcpy(p-12,&dead,4); }
            return p;
        }
        p+=(h[1]+3u)&~3u;
    }
    return p;
}
bool B29Queue::Next()
{
    if(!read || begin==end || uintptr_t(read)<uintptr_t(begin) || uintptr_t(read)>=uintptr_t(end) || consumed>=written)
        return false;
#if defined(_M_IX86)
    read=CallGameMethod<uint8_t*,0x0089F680>(&view,read);
#else
    view.begin=read;
    std::memcpy(&view.tag,read,4); std::memcpy(&view.bytes,read+4,4); std::memcpy(&view.count,read+8,4);
    view.records=read+12; view.end=view.records+view.bytes; read=view.end;
#endif
    ++consumed;
    // cmp written,consumed; sbb eax,eax; add eax,1: the last command
    // still returns true. Only a subsequent Next() returns false.
    return consumed<=written;
}
uint8_t* B29Queue::Reset()
{
    view={};
    pending=nullptr; sizeSlot=nullptr; payload=nullptr; written=0;
    pendingCount=0; read=nullptr; consumed=0;
    write=begin!=end ? begin : nullptr;
    return write;
}
B29Queue* B29Queue::Construct(void* a)
{
    begin=nullptr; end=nullptr; capacity=nullptr;
#if defined(_M_IX86)
    label=reinterpret_cast<const char*>(0x010313F4);
#else
    label="Audio/CommandQueue";
#endif
    // The embedded view at +10 is deliberately left untouched.
    allocator=a; write=nullptr; read=nullptr; pending=nullptr; sizeSlot=nullptr; payload=nullptr;
    written=0; state=0; pendingCount=0; consumed=0; marker=0xffffffffu; tail=0;
    return this;
}
uint8_t* B29Queue::Finish()
{
    auto* old=sizeSlot;
    if(old) {
        uint32_t size=uint32_t(uintptr_t(write)-uintptr_t(payload)-4);
        std::memcpy(old,&size,4);
        ++written;
        pendingCount=0; payload=nullptr; sizeSlot=nullptr; pending=nullptr;
        return old;
    }
    view={}; written=0; write=begin!=end ? begin : nullptr;
    read=nullptr; consumed=0; pendingCount=0; payload=nullptr; sizeSlot=nullptr; pending=nullptr;
    return write;
}
B29Queue* B29Queue::Swap(B29Queue* other)
{
    std::swap(begin,other->begin); std::swap(end,other->end); std::swap(capacity,other->capacity);
    std::swap(allocator,other->allocator); std::swap(write,other->write);
    if(write==end) write=begin;
    std::swap(pending,other->pending); std::swap(sizeSlot,other->sizeSlot); std::swap(payload,other->payload);
    std::swap(written,other->written); std::swap(state,other->state);
    return other;
}
}

namespace ts3::audio::b29hooks {
bool __fastcall F0089F850(B29Queue* self, void*) { return self->Next(); }
uint8_t* __fastcall F0089F8A0(B29Queue* self, void*) { return self->Reset(); }
bool __fastcall F0089F8E0(B29Records* self, void*, uint32_t tag, uint32_t* type, uint32_t* size) { return self->Contains(tag, type, size); }
bool __fastcall F0089F900(B29Records* self, void*, uint32_t tag, float* out) { return self->ReadFloat(tag, out); }
bool __fastcall F0089F980(B29Records* self, void*, uint32_t tag, uint32_t* out) { return self->ReadKey(tag, out); }
void* __fastcall F0089F9C0(B29Records* self, void*, uint32_t tag) { return self->ReadString(tag); }
bool __fastcall F0089F9F0(B29Records* self, void*, uint32_t tag, uint32_t* out) { return self->ReadWord(tag, out); }
void* __fastcall F0089FA30(B29Records* self, void*, uint32_t tag, uint32_t* size) { return self->ReadBlob(tag, size); }
bool __fastcall F0089FA70(B29Records* self, void*, uint32_t tag, float* out) { return self->ReadVector(tag, out); }
bool __fastcall F0089FAC0(B29Records* self, void*, uint32_t tag, float* out) { return self->ReadMatrix(tag, out); }
bool __fastcall F0089FB50(B29Records* self, void*, uint32_t tag, uint32_t low, uint32_t high) { return self->WriteKey(tag, low, high); }
uint8_t* __fastcall F0089FBD0(B29Records* self, void*, uint32_t tag) { return self->Erase(tag); }
B29Queue* __fastcall F0089FC20(B29Queue* self, void*, void* allocator) { return self->Construct(allocator); }
uint8_t* __fastcall F0089FC70(B29Queue* self, void*) { return self->Finish(); }
B29Queue* __fastcall F0089FCD0(B29Queue* self, void*, B29Queue* other) { return self->Swap(other); }
}
