#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::ui {

// A 4x4 row-major matrix plus a "kind" tag at +0x40 that selects the inverse form and
// records the running complexity of the accumulated product. Rows are 16-byte groups of
// floats consumed by the SSE helpers below. (x86 layout of the game object; the x64 core
// build has its own layout.)
struct MatrixAccumulator {
    float row[4][4];
    uint32_t kind;

    // TS3W.exe 0x00511B10: identity matrix with kind cleared.
    void SetIdentity();
    // TS3W.exe 0x00511C10: this = this * other (row-vector convention), kind = max.
    void MultiplyRight(const MatrixAccumulator* other);
    // TS3W.exe 0x00513410: this = other * this, kind = max.
    void MultiplyLeft(const MatrixAccumulator* other);
    // TS3W.exe 0x005125E0: rows 0-2 = diag(scale), row 3 = (translate.xyz, 1); kind = 1.
    // Returns the translate pointer (the game's EAX).
    const void* SetScaleTranslation(const float* translate, const float* scale);
    // TS3W.exe 0x005126D0: row3 += row3.w * (v.xyz, 0); kind 3 also moves rows 1-2 and
    // leaves row 0 untouched (the game adds a zero factor there). Returns v (EAX).
    const void* TranslateByRowW(const float* v);
    // TS3W.exe 0x00512770: row3 += v.x*row0 + v.y*row1 + v.z*row2. Returns v (EAX).
    const void* TranslateRowsBy(const float* v);
    // TS3W.exe 0x00512F60: in-place inverse selected by kind (1 affine, 2 general
    // 3x3+translation, 3 full). Returns the game's EAX (0 / a movemask / kind).
    int32_t Invert();
};
TS3_X86_OFFSET(MatrixAccumulator, kind, 0x40);
TS3_X86_SIZE(MatrixAccumulator, 0x44);

// Replace-hook shims for the entry points the game calls with the __thiscall member ABI:
// the replace patch jumps straight to these, so they must reproduce the original calling
// convention exactly (`this` in ECX, callee-cleaned stack arguments). `__fastcall` with a
// leading self argument models that on x86; the second parameter stands for the
// register-passed dummy the original never reads. x86-only build glue - the rewrite
// itself is portable and does not use these.
#if defined(_M_IX86)
void* __fastcall RebindListRecordsHookThunk(void* self, void*, void* transform, uint32_t tag);
void __fastcall MultiplyRightHookThunk(void* self, void*, const void* other);
const void* __fastcall SetScaleTranslationHookThunk(void* self, void*, const void* translate,
                                                   const void* scale);
const void* __fastcall TranslateByRowWHookThunk(void* self, void*, const void* v);
const void* __fastcall TranslateRowsByHookThunk(void* self, void*, const void* v);
int32_t __fastcall PresentFromLayoutHookThunk(void* self, void*);
#endif

// TS3W.exe 0x00513560 (__cdecl): component-wise packed 8-bit multiply with the game's
// (a*b+128 + ((a*b+128)>>8)) >> 8 rounding.
uint32_t MultiplyPackedColor(uint32_t left, uint32_t right);

// One node of the intrusive lists 0x0050B5F0 walks. `key` is compared against the
// transform pointer argument and `record`'s dword at +8 against the tag.
struct LinkRecordNode {
    LinkRecordNode* next;  // +0
    void* key;             // +4 (transform pointer)
    void* record;          // +8
};
TS3_X86_OFFSET(LinkRecordNode, key, 4);
TS3_X86_OFFSET(LinkRecordNode, record, 8);
TS3_X86_SIZE(LinkRecordNode, 0xC);

// The manager that owns a set of transform objects. 0x0050B5F0 walks the two list heads
// at +0x6A8/+0x6AC; 0x0050CB60 inserts into the list whose sentinel is at +0x18 and
// reads the byte flag at +0x30.
struct TransformManager {
    uint8_t pad0[0x18];
    void* listSentinelNext;  // +0x18 (+4 inside the sentinel at +0x18)
    uint8_t pad1[0x14];
    uint8_t flag30;  // +0x30
    uint8_t pad2[0x677];
    void* listHeadA;  // +0x6A8
    void* listHeadB;  // +0x6AC

    // TS3W.exe 0x0050B5F0: rebinds records of list nodes matching (transform, tag).
    void* RebindListRecords(void* transform, uint32_t tag);
    // TS3W.exe 0x0050CB60: applies/updates the transform object, recursing into children.
    int8_t UpdateTransform(void* transform, uint32_t flagsA, uint32_t flagsB);
};
TS3_X86_OFFSET(TransformManager, listSentinelNext, 0x18);
TS3_X86_OFFSET(TransformManager, flag30, 0x30);
TS3_X86_OFFSET(TransformManager, listHeadA, 0x6A8);
TS3_X86_OFFSET(TransformManager, listHeadB, 0x6AC);

// A UI transform object; only the fields this unit touches are named.
struct UiTransform {
    uint8_t pad0[4];
    void* iface;  // +0x04 (vtable pointer of the embedded interface)
    uint8_t pad1[0x18];
    void* nodePrev;  // +0x20 (manager list node)
    void* nodeNext;  // +0x24
    uint8_t pad2a[4];
    void* parent;  // +0x2C
    void* owner;   // +0x30
    uint8_t pad2[0xC];
    float x0;  // +0x40
    float y0;  // +0x44
    float x1;  // +0x48
    float y1;  // +0x4C
    uint8_t pad3[0x20];
    uint8_t flags70;  // +0x70
    uint8_t pad4[0x3F];
    MatrixAccumulator matrixB0;   // +0xB0
    uint8_t padM0[0xC];
    MatrixAccumulator matrix100;  // +0x100
    uint8_t padM1[0xC];
    MatrixAccumulator matrix150;  // +0x150
    uint8_t padM2[0xC];
    MatrixAccumulator matrix1A0;  // +0x1A0
    uint8_t padM3[0xC];
    uint32_t color1F0;  // +0x1F0
    uint32_t flag1F4;   // +0x1F4
    uint8_t flags1F8;   // +0x1F8
    uint8_t flags1F9;   // +0x1F9
    uint8_t pad4c[2];
    uint32_t color1FC;  // +0x1FC
    uint8_t pad4d[4];
    void* iface204;     // +0x204
    uint8_t pad6[8];
    void* childList;  // +0x210
    uint8_t pad7[0xE4];
    void* iface2F8;  // +0x2F8
    uint8_t pad8[0x10];
    float value30C;  // +0x30C
    float value310;  // +0x310
    uint8_t pad9[4];

    // TS3W.exe 0x00514340: copies matrixB0 into matrix100, inverts it, syncs a mode bit
    // and presents the visual. Returns the game's EAX (a changed-mask or a virtual result).
    int32_t PresentFromLayout();
    // TS3W.exe 0x00515330: PresentFromLayout() and report 1.
    int8_t ApplyToLayout(int32_t arg);
};
TS3_X86_OFFSET(UiTransform, iface, 0x04);
TS3_X86_OFFSET(UiTransform, nodePrev, 0x20);
TS3_X86_OFFSET(UiTransform, nodeNext, 0x24);
TS3_X86_OFFSET(UiTransform, parent, 0x2C);
TS3_X86_OFFSET(UiTransform, owner, 0x30);
TS3_X86_OFFSET(UiTransform, x0, 0x40);
TS3_X86_OFFSET(UiTransform, x1, 0x48);
TS3_X86_OFFSET(UiTransform, flags70, 0x70);
TS3_X86_OFFSET(UiTransform, matrixB0, 0xB0);
TS3_X86_OFFSET(UiTransform, matrix100, 0x100);
TS3_X86_OFFSET(UiTransform, matrix150, 0x150);
TS3_X86_OFFSET(UiTransform, matrix1A0, 0x1A0);
TS3_X86_OFFSET(UiTransform, color1F0, 0x1F0);
TS3_X86_OFFSET(UiTransform, flag1F4, 0x1F4);
TS3_X86_OFFSET(UiTransform, flags1F8, 0x1F8);
TS3_X86_OFFSET(UiTransform, flags1F9, 0x1F9);
TS3_X86_OFFSET(UiTransform, color1FC, 0x1FC);
TS3_X86_OFFSET(UiTransform, iface204, 0x204);
TS3_X86_OFFSET(UiTransform, childList, 0x210);
TS3_X86_OFFSET(UiTransform, iface2F8, 0x2F8);
TS3_X86_OFFSET(UiTransform, value30C, 0x30C);
TS3_X86_OFFSET(UiTransform, value310, 0x310);

// TS3W.exe 0x00515380 (__cdecl, object in the first stack argument): PresentFromLayout
// and report 1.
int8_t RefreshTransformThunk(UiTransform* self);
// TS3W.exe 0x0053B800 (__cdecl): forwards to 0x00515330, returning its 1.
int8_t ApplyTransformThunk(UiTransform* self, int32_t arg);
// TS3W.exe 0x0077EA70 (__cdecl): tail-forwards to 0x00514340 and returns its result.
int8_t PresentTransformThunk(UiTransform* self);
// TS3W.exe 0x005462B0 (__cdecl): stamps both +0x30C/+0x310 with 9.0f, feeds them to the
// +0x2F8 interface, then presents through the +0x04 interface.
int8_t PrepareAndPresentTransform(UiTransform* self);

}  // namespace ts3::ui