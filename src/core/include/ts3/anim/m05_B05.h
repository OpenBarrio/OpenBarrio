#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::b05_impl {
// Only the observed evaluator prefix; links remain native pointers.
struct NodePrefix { void* vtable; void* next; void* previous; void* first; void* last; void* parent; uint32_t value18; float weight, previousWeight; };
struct Range { unsigned char* begin; unsigned char* end; unsigned char* capacity; void* descriptor; };
struct Records { NodePrefix base; Range records; };
struct Retained { NodePrefix base; ResourcePrefix* resource; };
struct Reader { int32_t offset; uint32_t value4,value8; unsigned char* buffer; uint32_t value10; uint8_t failed; };
struct Writer { int32_t offset; uint32_t value4,value8,valueC; unsigned char* buffer; uint32_t value14; uint8_t failed; };
TS3_X86_OFFSET(Records,records,0x24);
TS3_X86_OFFSET(Retained,resource,0x24);
TS3_X86_OFFSET(Reader,buffer,0xc);
TS3_X86_OFFSET(Writer,buffer,0x10);
// TS3W.exe 0x0055BCC0: return the position-record evaluator type ID.
uint32_t TypeBCC0(void*);
// TS3W.exe 0x0055BCD0: query position-record or base evaluator interface.
void* QueryBCD0(void*,uint32_t);
// TS3W.exe 0x0055BD40: insert a stride32 record, copying only its first 20 bytes.
uintptr_t InsertBD40(Range*,unsigned char*,const unsigned char*);
// TS3W.exe 0x0055BEC0: replace position records from the buffered reader.
uintptr_t LoadBEC0(Records*,Reader*);
// TS3W.exe 0x0055BFD0: return the evaluator type ID.
uint32_t TypeBFD0(void*);
// TS3W.exe 0x0055BFE0: query derived or base evaluator interface.
void* QueryBFE0(void*,uint32_t);
// TS3W.exe 0x0055CFF0: return the retained-pose evaluator type ID.
uint32_t TypeCFF0(void*);
// TS3W.exe 0x0055D000: query retained-pose or base evaluator interface.
void* QueryD000(void*,uint32_t);
// TS3W.exe 0x0055D030: retain replacement before releasing old; restore refs to one before deletion.
uintptr_t SetD030(Retained*,ResourcePrefix*);
// TS3W.exe 0x0055D0C0: copy SRTs and optionally allocate/copy vertex weights; null source is a no-op.
void CopyD0C0(LocalPoseBuffer*,const RigPose*);
// TS3W.exe 0x0055D220: serialize records, quaternion before the three translation lanes.
uintptr_t SaveD220(const Records*,Writer*);
// TS3W.exe 0x0055D3E0: return the transform-record evaluator type ID.
uint32_t TypeD3E0(void*);
// TS3W.exe 0x0055D3F0: query transform-record or base evaluator interface.
void* QueryD3F0(void*,uint32_t);
// TS3W.exe 0x0055D460: insert stride48 records, preserving destination padding.
uintptr_t InsertD460(Range*,unsigned char*,const unsigned char*);
// TS3W.exe 0x0055D610: load quaternion, three translation lanes and index; translation W is unspecified.
uintptr_t LoadD610(Records*,Reader*);
}

// Hookgen's thiscall adapters require a class member. These stateless ABI views
// forward to the independently differential-tested implementations above.
namespace ts3::anim {
struct b05 {
    uint32_t TypeBCC0() { return b05_impl::TypeBCC0(this); }
    uint32_t QueryBCD0(uint32_t id) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b05_impl::QueryBCD0(this,id))); }
    uint32_t InsertBD40(void* p,void* v) { return static_cast<uint32_t>(b05_impl::InsertBD40(reinterpret_cast<b05_impl::Range*>(this),static_cast<unsigned char*>(p),static_cast<const unsigned char*>(v))); }
    uint32_t LoadBEC0(void* s) { return static_cast<uint32_t>(b05_impl::LoadBEC0(reinterpret_cast<b05_impl::Records*>(this),static_cast<b05_impl::Reader*>(s))); }
    uint32_t TypeBFD0() { return b05_impl::TypeBFD0(this); }
    uint32_t QueryBFE0(uint32_t id) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b05_impl::QueryBFE0(this,id))); }
    uint32_t TypeCFF0() { return b05_impl::TypeCFF0(this); }
    uint32_t QueryD000(uint32_t id) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b05_impl::QueryD000(this,id))); }
    uint32_t SetD030(void* p) { return static_cast<uint32_t>(b05_impl::SetD030(reinterpret_cast<b05_impl::Retained*>(this),static_cast<ResourcePrefix*>(p))); }
    void CopyD0C0(void* p) { b05_impl::CopyD0C0(reinterpret_cast<LocalPoseBuffer*>(this),static_cast<const RigPose*>(p)); }
    uint32_t SaveD220(void* s) { return static_cast<uint32_t>(b05_impl::SaveD220(reinterpret_cast<const b05_impl::Records*>(this),static_cast<b05_impl::Writer*>(s))); }
    uint32_t TypeD3E0() { return b05_impl::TypeD3E0(this); }
    uint32_t QueryD3F0(uint32_t id) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b05_impl::QueryD3F0(this,id))); }
    uint32_t InsertD460(void* p,void* v) { return static_cast<uint32_t>(b05_impl::InsertD460(reinterpret_cast<b05_impl::Range*>(this),static_cast<unsigned char*>(p),static_cast<const unsigned char*>(v))); }
    uint32_t LoadD610(void* s) { return static_cast<uint32_t>(b05_impl::LoadD610(reinterpret_cast<b05_impl::Records*>(this),static_cast<b05_impl::Reader*>(s))); }
};
}
