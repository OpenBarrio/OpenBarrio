#include "ts3/scratch/clu_4e3f70_Scratch.h"
namespace ts3::scratch {
void* Free4E3F70(void* block) {
    if (!block) return nullptr;
#if defined(_M_IX86)
    auto* allocator = *reinterpret_cast<void**>(0x011CB864);
    return CallGameMethod<void*, 0x004E5920>(allocator, block);
#else
    NotRewritten("Scratch allocator global 0x011CB864");
#endif
}
template<unsigned Address> static void* GameVtable() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(static_cast<uintptr_t>(Address));
#else
    NotRewritten("Scratch game vtable");
#endif
}
// 0x004E8B80: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete004E8B80(uint32_t flags) {
    CallGameMethod<void, 0x4e8a30>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x004F27E0: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete004F27E0(uint32_t flags) {
    CallGameMethod<void, 0x4f2720>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x004F28D0: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete004F28D0(uint32_t flags) {
    CallGameMethod<void, 0x4f2780>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x00502D50: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete00502D50(uint32_t flags) {
    vtable = GameVtable<0xfb9b38>();
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x00502DC0: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete00502DC0(uint32_t flags) {
    vtable = GameVtable<0xfb9b80>();
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x00502DE0: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete00502DE0(uint32_t flags) {
    vtable = GameVtable<0xfb9c58>();
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x00505D80: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete00505D80(uint32_t flags) {
    vtable = GameVtable<0xfba248>();
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x00576C90: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete00576C90(uint32_t flags) {
    CallGameMethod<void, 0x5659d0>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x0058DA90: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete0058DA90(uint32_t flags) {
    CallGameMethod<void, 0x4d89b0>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x0058F7E0: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete0058F7E0(uint32_t flags) {
    CallGameMethod<void, 0x4065a0>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x00599140: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete00599140(uint32_t flags) {
    vtable = GameVtable<0xfd9748>();
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x0059FF90: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete0059FF90(uint32_t flags) {
    vtable = GameVtable<0xfddf08>();
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x0060B990: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete0060B990(uint32_t flags) {
    CallGameMethod<void, 0x60b5b0>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x00613F60: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete00613F60(uint32_t flags) {
    vtable = GameVtable<0xfe8144>();
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x006D45D0: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete006D45D0(uint32_t flags) {
    CallGameMethod<void, 0x6d4090>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x0070BE90: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete0070BE90(uint32_t flags) {
    vtable = GameVtable<0xffb294>();
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x00728740: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete00728740(uint32_t flags) {
    CallGameMethod<void, 0x728280>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x0076AD90: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete0076AD90(uint32_t flags) {
    CallGameMethod<void, 0x76abd0>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x007B2300: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete007B2300(uint32_t flags) {
    CallGameMethod<void, 0x7b2000>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x007C8C20: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete007C8C20(uint32_t flags) {
    CallGameMethod<void, 0x7c8b10>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x007CBAD0: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete007CBAD0(uint32_t flags) {
    CallGameMethod<void, 0x7cba50>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x007DA2B0: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete007DA2B0(uint32_t flags) {
    CallGameMethod<void, 0x472050>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x007FE330: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete007FE330(uint32_t flags) {
    CallGameMethod<void, 0x7f7070>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x00800070: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete00800070(uint32_t flags) {
    CallGameMethod<void, 0x7ffc10>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x0081CCB0: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete0081CCB0(uint32_t flags) {
    CallGameMethod<void, 0x81cbe0>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x00877350: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete00877350(uint32_t flags) {
    CallGameMethod<void, 0x877290>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x009C75C0: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete009C75C0(uint32_t flags) {
    CallGameMethod<void, 0x9c6c90>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x009D7610: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete009D7610(uint32_t flags) {
    vtable = GameVtable<0x1051a74>();
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x009D7630: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete009D7630(uint32_t flags) {
    vtable = GameVtable<0x1051aa8>();
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x009EF360: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete009EF360(uint32_t flags) {
    vtable = GameVtable<0x113863c>();
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x009EFFF0: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete009EFFF0(uint32_t flags) {
    vtable = GameVtable<0x1054f08>();
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x009F1FC0: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete009F1FC0(uint32_t flags) {
    CallGameMethod<void, 0x9f1a70>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x00C25B20: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete00C25B20(uint32_t flags) {
    CallGameMethod<void, 0xc25920>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// 0x00EED600: thiscall, ret 4; all other flag bits are ignored.
void* DeletingObject::Delete00EED600(uint32_t flags) {
    CallGameMethod<void, 0xeebd90>(this);
    if (flags & 1) Free4E3F70(this);
    return this;
}
// Unsigned subtraction avoids undefined unrelated-pointer subtraction; interpret the
// resulting machine-width bit pattern as signed, just like the x86 CMP/JLE.
static intptr_t Capacity(void* begin, void* limit, uintptr_t mask) {
    return static_cast<intptr_t>((reinterpret_cast<uintptr_t>(limit) -
        reinterpret_cast<uintptr_t>(begin)) & mask);
}
// 0x0087F5A0: thiscall, ret.
void* Cleanup87F5A0::Release() {
    auto* block = span.begin;
    if (Capacity(block, span.limit, ~uintptr_t(1)) > 2 && block) return Free4E3F70(block);
    return block;
}
// 0x00883A80: thiscall, ret.
void* Cleanup883A80::Release() {
    auto* block = span.begin;
    if (Capacity(block, span.limit, ~uintptr_t(1)) > 2 && block) return Free4E3F70(block);
    return block;
}
// 0x00945440: argument comes from the stack, NOT ECX.
void* TS3_SCRATCH_STDCALL Release945440(ScratchSpan* span) {
    auto* block = span->begin;
    if (Capacity(block, span->limit, ~uintptr_t(0)) > 1 && block) return Free4E3F70(block);
    return block;
}
// 0x00945A30: argument comes from the stack, NOT ECX.
void* TS3_SCRATCH_STDCALL Release945A30(ExternalStorage* storage) {
    auto* block = storage->block;
    if (block && block != storage->internal) return Free4E3F70(block);
    return block;
}
// 0x00C16FD0: thiscall, ret; preserves the dangling field.
void* CleanupC16FD0::Release() {
    return block ? Free4E3F70(block) : nullptr;
}
} // namespace ts3::scratch
