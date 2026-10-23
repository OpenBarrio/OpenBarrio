// Reimplementation of unit cls_702490_LotRenderer (subsystem LotRenderer) from the
// original TS3W.exe 1.67.2 disassembly in the original disassembly/
// (00702490.asm, 00702A90.asm, 00702B40.asm, 0070D4E0.asm, 007249D0.asm,
// 0093E9E0.asm, 0093EA40.asm, 0093EAA0.asm - the .asm files are the source of truth:
// the Hex-Rays output in decompile/ disagreed on two quad-argument orders and on the
// 0x702A90 clamp comparisons, and the assembly won). Every dependency outside this
// unit goes through the ts3/layout.h bridge (CallGameFunction/CallGameMethod/
// CallGameVirtual), which aborts on x64 where there is no game code to call.
//
// Return-contract note: 0x0070D4E0, 0x007249D0 and both 0x00702490 return paths set
// AL only; the C++ return types below are the zero-extended byte, and the
// differential tests compare the low byte only.

#include "ts3/lotrenderer/cls_702490_LotRenderer.h"

#include <cstring>

namespace ts3::lotrenderer {
namespace {

#if defined(_M_IX86)

float BitsToFloat(uint32_t bits) {
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// Image-resident float constants (read at runtime, they live in .rdata):
//   0x1071AE0 = 1/255   0x106D6CC = 0.01f   0x107A538 = 1.0f   0xF98964 = Flat
constexpr uintptr_t kInv255Addr = 0x1071AE0;
constexpr uintptr_t kThresholdAddr = 0x106D6CC;
constexpr uintptr_t kOneAddr = 0x107A538;
constexpr uintptr_t kFlatAddr = 0xF98964;

// 0x00702B40 registration table, extracted from the push order in 00702B40.asm
// (67 entries; every call passes (this, hash, second) to vtable slot 11 - byte
// offset 0x2C - of the lot object, with second = 0xFFFFD8F1 except the two zeros).
struct LotRegistration {
    uint32_t hash;
    uint32_t second;
};

constexpr LotRegistration kRegistrations[] = {
    {0xB3436C13u, 0xFFFFD8F1u},
    {0x2F704CABu, 0xFFFFD8F1u},
    {0x732F79FDu, 0xFFFFD8F1u},
    {0x5358A028u, 0xFFFFD8F1u},
    {0x542246A3u, 0xFFFFD8F1u},
    {0x9375C04Fu, 0xFFFFD8F1u},
    {0x057F931Au, 0xFFFFD8F1u},
    {0x0A6EBD0Au, 0xFFFFD8F1u},
    {0x05B70C75u, 0xFFFFD8F1u},
    {0x05B70C76u, 0xFFFFD8F1u},
    {0x095448E2u, 0xFFFFD8F1u},
    {0xCB3BB2C9u, 0xFFFFD8F1u},
    {0x3227A26Eu, 0xFFFFD8F1u},
    {0xFA442E78u, 0xFFFFD8F1u},
    {0x7727AD2Eu, 0xFFFFD8F1u},
    {0x03DFE432u, 0xFFFFD8F1u},
    {0x3406565Fu, 0xFFFFD8F1u},
    {0x950BA4A7u, 0xFFFFD8F1u},
    {0x04878BC4u, 0xFFFFD8F1u},
    {0x0DCCD255u, 0xFFFFD8F1u},
    {0x048A0F45u, 0xFFFFD8F1u},
    {0x04D18170u, 0xFFFFD8F1u},
    {0x04A4CFE9u, 0xFFFFD8F1u},
    {0x04AE06E0u, 0xFFFFD8F1u},
    {0x063BD2A3u, 0xFFFFD8F1u},
    {0x0B581957u, 0xFFFFD8F1u},
    {0x56B00C87u, 0xFFFFD8F1u},
    {0x048A2325u, 0xFFFFD8F1u},
    {0x07A02860u, 0xFFFFD8F1u},
    {0x5E7CA288u, 0xFFFFD8F1u},
    {0xF33E398Bu, 0xFFFFD8F1u},
    {0x73477A1Cu, 0xFFFFD8F1u},
    {0x535064E7u, 0xFFFFD8F1u},
    {0x2BE20162u, 0xFFFFD8F1u},
    {0x6A4479F1u, 0xFFFFD8F1u},
    {0x85BBE375u, 0xFFFFD8F1u},
    {0x46FA1EEAu, 0xFFFFD8F1u},
    {0x76211120u, 0xFFFFD8F1u},
    {0xF34B709Cu, 0xFFFFD8F1u},
    {0x934CB2A9u, 0xFFFFD8F1u},
    {0x049CB815u, 0xFFFFD8F1u},
    {0xE92EBE5Fu, 0xFFFFD8F1u},
    {0x049CB828u, 0xFFFFD8F1u},
    {0x049CB899u, 0xFFFFD8F1u},
    {0x7357350Du, 0xFFFFD8F1u},
    {0x3359DC3Cu, 0xFFFFD8F1u},
    {0x0976BAF2u, 0xFFFFD8F1u},
    {0x0488D99Cu, 0xFFFFD8F1u},
    {0x70F3BE97u, 0xFFFFD8F1u},
    {0x05BFF1A9u, 0xFFFFD8F1u},
    {0x60B3D82Eu, 0xFFFFD8F1u},
    {0x0E5087EDu, 0xFFFFD8F1u},
    {0x099B63B3u, 0xFFFFD8F1u},
    {0xECFF5D63u, 0xFFFFD8F1u},
    {0x0E092878u, 0xFFFFD8F1u},
    {0xDB32B443u, 0xFFFFD8F1u},
    {0x827917CAu, 0xFFFFD8F1u},
    {0x59EF8B87u, 0xFFFFD8F1u},
    {0x1C8880A3u, 0xFFFFD8F1u},
    {0x0D8A7ECCu, 0xFFFFD8F1u},
    {0x0428E0A3u, 0x00000000u},
    {0x0428E0A8u, 0x00000000u},
    {0x04891B02u, 0xFFFFD8F1u},
    {0x9286833Au, 0xFFFFD8F1u},
    {0xC415B92Du, 0xFFFFD8F1u},
    {0xFC6AA1FEu, 0xFFFFD8F1u},
    {0xBA89B123u, 0xFFFFD8F1u},
};

#endif  // _M_IX86

}  // namespace

uint8_t LotMatchView::IsMatch70D4E0(uint32_t a2, uint32_t a3) const {
#if defined(_M_IX86)
    // 0x0070D4E0
    //           cmp [ecx+0x1b4],[esp+8] / jne; mov al,1; ret 8.  AL only.
    if (enabled4F8 == 0) {
        return 0;
    }
    if (field1B0 != a2) {
        return 0;
    }
    if (field1B4 != a3) {
        return 0;
    }
    return 1;
#else
    (void)a2;
    (void)a3;
    ts3::AbortNotRewritten("cls_702490_LotRenderer 0x0070D4E0");
    return 0;
#endif
}

uint8_t LotFlagView::StoreFlag7249D0(uint8_t value) {
#if defined(_M_IX86)
    // 0x007249D0
    flag8 = value;
    return value;
#else
    (void)value;
    ts3::AbortNotRewritten("cls_702490_LotRenderer 0x007249D0");
    return 0;
#endif
}

uint8_t LotRenderer::ApplyEntryColor702490(uint64_t key, const LotParams* params) {
#if defined(_M_IX86)
    // Prologue order (00702490.asm): reads the flag byte, then the +0 float, copies
    // the 8-byte key onto the frame, decodes rgb, then does the find.
    const uint8_t flag4 = params->flag4;
    const float f0 = BitsToFloat(params->f0Bits);

    // 7024B7: 6FDFB0(&out, &keyCopy) - "this" is the map header at renderer+4,
    // callee cleans 8.  Assumption (documented): the find reads the key without
    // writing it back, so the key copy still holds the key afterwards.
    void* findOut[4] = {};
    uint64_t keyCopy = key;
    CallGameMethod<void, 0x6FDFB0>(&head04, findOut, &keyCopy);

    // 7024D2: three miss paths return 0 (AL only).
    void* found = findOut[0];
    if (found == sentinel1028) {
        return 0;
    }
    if (found == nullptr) {
        return 0;
    }
    LotEntryNode* const node1 = reinterpret_cast<LotEntryNode*>(
        reinterpret_cast<uint8_t*>(found) - 8);   // add edi,-8
    if (node1 == nullptr) {                        // found == 8
        return 0;
    }

    // 7024F3: decode rgb (byte order from the asm: movzx esi,[ebx+0xe]=byte2 first,
    // then +0xd=byte1, then +0xc=byte0).  byte*K with K = float @0x1071AE0.
    const uint32_t rgb = params->rgbC;
    const float inv255 = *reinterpret_cast<const float*>(kInv255Addr);
    const float r = static_cast<float>((rgb >> 16) & 0xFFu) * inv255;
    const float g = static_cast<float>((rgb >> 8) & 0xFFu) * inv255;
    const float b = static_cast<float>(rgb & 0xFFu) * inv255;

    // 702522: comiss [0x106D6CC](0.01f), f0; jbe branchA.  jbe is taken when the
    // threshold is <= f0 ordered OR unordered, i.e. branch A runs when
    // !(threshold > f0) - NaN takes branch A.
    const float threshold = *reinterpret_cast<const float*>(kThresholdAddr);

    if (!(threshold > f0)) {
        // ---- branch A (f0 >= 0.01f, or NaN) ----
        uint32_t staleKeyDword = static_cast<uint32_t>(key);  // key copy at local +0x40
        if (flag4 != 0) {
            // 702639: 6FDCC0(field8, &v24, &v23, &v22) cdecl, callee cleans 0x10.
            uint32_t v24 = 0, v23 = 0, v22 = 0;
            CallGameFunction<void, 0x6FDCC0>(params->field8, &v24, &v23, &v22);
            // 702651: quad = {v24, v23, v22, node1+0x1D0.w} -> 6FD4D0(node1, quad),
            // thiscall, callee cleans 8.  The v24 slot reuses the old key copy.
            const float quad[4] = {BitsToFloat(v24), BitsToFloat(v23),
                                   BitsToFloat(v22), node1->q1D0.w};
            CallGameMethod<void, 0x6FD4D0>(node1, quad);
            staleKeyDword = v24;
        }
        // 7026A6: byte 0 of the stale dword is zeroed in place before it becomes
        // 0x7018B0's third argument (EA quirk kept - only the low byte is cleared).
        const uint32_t arg3 = staleKeyDword & 0xFFFFFF00u;

        // 7018B0(this = &c10F0, &out, &key, arg3) thiscall, callee cleans 12.
        // out: bytes 0..3 = node pointer, byte 4 = found flag.
        uint8_t out2[16] = {};
        CallGameMethod<void, 0x7018B0>(&c10F0, out2, &key, arg3);
        void* node2Raw = nullptr;
        std::memcpy(&node2Raw, out2, sizeof node2Raw);
        const bool found2 = out2[4] != 0;
        LotEntryNode* const node2 = reinterpret_cast<LotEntryNode*>(node2Raw);

        // 7026C9-702741: three unconditional quad writes.  EA quirk kept: there is
        // NO null check on 7018B0's node - the original writes through it blindly.
        const float flat = *reinterpret_cast<const float*>(kFlatAddr);
        node2->q60 = {flat, flat, flat, node2->q60.w};
        node2->q20 = {r, g, b, node2->q20.w};
        node2->q50 = {f0, f0, f0, node2->q50.w};

        if (found2 || flag4 != 0) {
            // Flag path: +0x30 and +0x40 take a copy of node1+0x1D0, then +0x20 is
            // rebuilt as {r,g,b,node1+0x1D0.w} from the quad just written.
            const LotQuad src = node1->q1D0;
            node2->q30 = src;
            node2->q40 = src;
            node2->q20 = {r, g, b, src.w};
        } else {
            // Blend path: +0x30 = {old+0x40.xyz, old+0x30.w} (the reads happen
            // after the three unconditional writes, but +0x30/+0x40 are untouched).
            const LotQuad old2 = node2->q40;
            const LotQuad old1 = node2->q30;
            node2->q30 = {old2.x, old2.y, old2.z, old1.w};
        }
        return 1;
    }

    // ---- branch B (f0 strictly below the threshold) ----
    // 702552: quad = {r, g, b, node1+0x1D0.w} -> 6FD4D0(node1, quad).
    {
        const float quad[4] = {r, g, b, node1->q1D0.w};
        CallGameMethod<void, 0x6FD4D0>(node1, quad);
    }
    // 7025A4: 6FE020(this = &c10F0, &out, &key) thiscall, callee cleans 8.
    void* outB[4] = {};
    CallGameMethod<void, 0x6FE020>(&c10F0, outB, &key);
    LotEntryNode* const nodeB = reinterpret_cast<LotEntryNode*>(outB[0]);
    // 702610: end iterator == &renderer+0x10F4, which on x86 is the address of the
    // f1 slot of c10F0 (base 0x10F0 + 4).
    if (nodeB == reinterpret_cast<LotEntryNode*>(static_cast<void*>(&c10F0.f1))) {
        return 1;
    }
    nodeB->q20 = {r, g, b, nodeB->q20.w};
    nodeB->q30 = {r, g, b, nodeB->q30.w};
    nodeB->q40 = {r, g, b, nodeB->q40.w};
    return 1;
#else
    (void)key;
    (void)params;
    ts3::AbortNotRewritten("cls_702490_LotRenderer 0x00702490");
    return 0;
#endif
}

int32_t LotRenderer::ApplyParams702A90(uint32_t a1, uint32_t a2, const LotParams* params) {
#if defined(_M_IX86)
    // 00702A90.asm: two byte->float decodes, a two-step pointer-select clamp, then
    // the tail call to game method 0x702270 (callee cleans 0x18 = 6 stack args).
    const float inv255 = *reinterpret_cast<const float*>(kInv255Addr);
    // v3 = byte at block +0xF (rgbC top), v4 = byte at block +0xB (field8 top).
    const float v3 = static_cast<float>((params->rgbC >> 24) & 0xFFu) * inv255;
    const float v4 = static_cast<float>((params->field8 >> 24) & 0xFFu) * inv255;
    const float f0 = BitsToFloat(params->f0Bits);
    // The original passes the +4 flag as the low byte of a dword whose upper three
    // bytes are caller stack garbage; the rewrite zero-extends the byte (documented
    // EA quirk in the unit header).
    const uint32_t flagDword = params->flag4;

    // Clamp with the original's pointer-select semantics (comiss/ja: each bound is
    // applied only when the compare is ordered, so NaN keeps the running value).
    // `volatile` keeps the compiler from fusing the compares into minss/maxss, whose
    // NaN rules differ (see the project notes.
    const float one = *reinterpret_cast<const float*>(kOneAddr);
    volatile float pick = v3;
    if (0.0f > pick) {
        pick = 0.0f;
    }
    if (pick > one) {
        pick = one;
    }
    const float pickFinal = pick;  // plain read of the clamped value

    return CallGameMethod<int32_t, 0x702270>(this, a1, a2, pickFinal, f0, flagDword, v4);
#else
    (void)a1;
    (void)a2;
    (void)params;
    ts3::AbortNotRewritten("cls_702490_LotRenderer 0x00702A90");
    return 0;
#endif
}

void LotRenderer::Teardown702B40() {
#if defined(_M_IX86)
    // (1) pseudo-vtable swap.
    vtable00 = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FFA1F4u));

    // (2) lot object from 0x44C470 (leaf reading the global at 0x11B1378); skip all
    // 67 registrations when it is null.
    void* const lot = CallGameFunction<void*, 0x44C470>();
    if (lot != nullptr) {
        for (const LotRegistration& reg : kRegistrations) {
            // vtable byte offset 0x2C = slot 11; args (this, hash, second), callee
            // cleans 0xC.
            CallGameVirtual<void, 11>(lot, this, reg.hash, reg.second);
        }
    }

    // (3) element loop: re-checked against +0x102C before every iteration.
    while (active102C != 0) {
        // 7030A6-7030F2: element select and iterator pair run the same unbounded
        // dword scan from head08 (EA quirk kept: no range guard - a container that
        // empties while the guard stays nonzero keeps scanning dwords past +0x10).
        void* first = head04;
        void** slot = &head04;
        if (first == nullptr) {
            slot = &head08;
            first = head08;
            if (first == nullptr) {
                slot = reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(&head08) + 4);
                while (*slot == nullptr) {
                    slot += 1;
                }
                first = *slot;
            }
        }
        // elem = found - 8 (7030C1); the dead je at 7030BF keeps the null branch.
        void* elem = (first != nullptr)
                         ? static_cast<void*>(reinterpret_cast<uint8_t*>(first) - 8)
                         : nullptr;

        // 6FDF40(this = &head04, out16, first, slot) thiscall, callee cleans 0xC
        // (6FDF40 ends in the original instruction sequence: three stack dwords).  out16 is written by the
        // callee and never read afterwards (dead, kept for call-order fidelity).
        void* out16[4] = {};
        CallGameMethod<void, 0x6FDF40>(&head04, out16, first, slot);

        // 703100: 16 zeroed bytes for the slot-14 call.
        void* v14[4] = {nullptr, nullptr, nullptr, nullptr};
        CallGameVirtual<void, 14>(elem, v14);

        // 70311E-7031F4: name selection - four slot-3 property queries feeding
        // strncpy/strcat into a local 0x80-byte buffer that is never read afterwards
        // (dead buffer, every virtual call kept for call-order fidelity).
        char nameBuf[0x80] = {};
        const auto query = [elem](uint32_t hash) -> void* {
            return CallGameVirtual<void*, 3>(elem, hash);
        };
        void* prop = query(0xE92EBE5Fu);
        if (prop != nullptr) {
            CallGameFunction<void, 0x4FDD30>(nameBuf, reinterpret_cast<const void*>(0x00FFA1E0u), 0x80u);
        } else if ((prop = query(0x0499DC66u)) != nullptr) {
            CallGameFunction<void, 0x4FDD30>(nameBuf, reinterpret_cast<const void*>(0x00FF99A8u), 0x80u);
        } else if ((prop = query(0x017F1D88u)) != nullptr) {
            CallGameFunction<void, 0x4FDD30>(nameBuf, reinterpret_cast<const void*>(0x00FFA1D4u), 0x80u);
        } else if ((prop = query(0x938D7230u)) != nullptr) {
            CallGameFunction<void, 0x4FDD30>(nameBuf, reinterpret_cast<const void*>(0x00FFA1C4u), 0x80u);
            // 7031B6: the same query again, then the [prop+0x210]->[+0x18] string
            // strcat (each null just skips to slot 1).
            void* const prop2 = query(0x938D7230u);
            if (prop2 != nullptr) {
                void* inner = *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(prop2) + 0x210);
                if (inner != nullptr) {
                    const void* str = *reinterpret_cast<const void**>(reinterpret_cast<uint8_t*>(inner) + 0x18);
                    CallGameFunction<void, 0x4FE080>(nameBuf, str, 0x80u);
                }
            }
        } else {
            CallGameFunction<void, 0x4FDD30>(nameBuf, reinterpret_cast<const void*>(0x00FCB4E8u), 0x80u);
        }

        CallGameVirtual<void, 1>(elem);
    }

    // (4) cleanup tail, exact block order from 00703209-0070338D.  The counted
    // blocks: clear(base, +4, +8), then +0xC = 0 and (count > 1u) free(+4) through
    // 0x4E3F70 cdecl with callee-cleans 4.  ebx stays 1 / ebp stays 0 through all of
    // them in the original.
    const auto counted = [](auto& block, auto clearCall, auto freeFn) {
        clearCall(block);
        const uint32_t count = block.f2;
        block.f3 = 0;
        if (count > 1u) {
            freeFn(block.f1);
        }
    };

    counted(c1188, [](LotContainer& b) {
        CallGameMethod<void, 0x5A3AE0>(&b, b.f1, b.f2);
    }, [](void* p) { CallGameFunction<void, 0x4E3F70>(p); });

    CallGameMethod<void, 0x6A0850>(&c116C, c116C.f3);

    counted(c1148, [](LotContainer& b) {
        CallGameMethod<void, 0xD2BC10>(&b, b.f1, b.f2);
    }, [](void* p) { CallGameFunction<void, 0x4E3F70>(p); });

    counted(c1128, [](LotContainer& b) {
        CallGameMethod<void, 0x700C60>(&b, b.f1, b.f2);
    }, [](void* p) { CallGameFunction<void, 0x4E3F70>(p); });

    CallGameMethod<void, 0x6A0850>(&c110C, c110C.f3);
    CallGameMethod<void, 0x6A0850>(&c10F0, c10F0.f3);

    counted(c1094, [](LotContainer& b) {
        CallGameMethod<void, 0x82BE80>(&b, b.f1, b.f2);
    }, [](void* p) { CallGameFunction<void, 0x4E3F70>(p); });

    counted(c1074, [](LotContainer& b) {
        CallGameMethod<void, 0x6258F0>(&b, b.f1, b.f2);
    }, [](void* p) { CallGameFunction<void, 0x4E3F70>(p); });

    counted(c1054, [](LotContainer& b) {
        CallGameMethod<void, 0x6258F0>(&b, b.f1, b.f2);
    }, [](void* p) { CallGameFunction<void, 0x4E3F70>(p); });

    counted(c1034, [](LotContainer& b) {
        CallGameMethod<void, 0x6258F0>(&b, b.f1, b.f2);
    }, [](void* p) { CallGameFunction<void, 0x4E3F70>(p); });

    // 70338D: closing pseudo-vtable swap.
    vtable00 = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FB2CBCu));
#else
    ts3::AbortNotRewritten("cls_702490_LotRenderer 0x00702B40");
#endif
}

int32_t Pump93E9E0(int32_t a1, int32_t a2) {
#if defined(_M_IX86)
    // 0093E9E0: NO null check on 6FDE10's result (EA quirk kept - compare with
    // Pump93EA40); every other null path returns 0 (the failed pointer read as 0).
    void* const obj = CallGameFunction<void*, 0x6FDE10>();
    void* const res = CallGameMethod<void*, 0x6FE3B0>(obj, a1, a2);
    if (res == nullptr) {
        return 0;
    }
    void* const v = CallGameVirtual<void*, 3>(res, 0x017F1D88u);
    if (v == nullptr) {
        return 0;
    }
    void* const a = CallGameMethod<void*, 0x6F4A50>(v);
    if (a == nullptr) {
        return 0;
    }
    CallGameMethod<int32_t, 0x573300>(a);
    CallGameMethod<int32_t, 0x6FC9E0>(v);
    return CallGameMethod<int32_t, 0x6FAC70>(v);
#else
    (void)a1;
    (void)a2;
    ts3::AbortNotRewritten("cls_702490_LotRenderer 0x0093E9E0");
    return 0;
#endif
}

int32_t Pump93EA40(int32_t a1, int32_t a2) {
#if defined(_M_IX86)
    // 0093EA40: identical chain WITH the 6FDE10 null check.
    void* const obj = CallGameFunction<void*, 0x6FDE10>();
    if (obj == nullptr) {
        return 0;
    }
    void* const res = CallGameMethod<void*, 0x6FE3B0>(obj, a1, a2);
    if (res == nullptr) {
        return 0;
    }
    void* const v = CallGameVirtual<void*, 3>(res, 0x017F1D88u);
    if (v == nullptr) {
        return 0;
    }
    void* const a = CallGameMethod<void*, 0x6F4A50>(v);
    if (a == nullptr) {
        return 0;
    }
    CallGameMethod<int32_t, 0x573300>(a);
    CallGameMethod<int32_t, 0x6FC9E0>(v);
    return CallGameMethod<int32_t, 0x6FAC70>(v);
#else
    (void)a1;
    (void)a2;
    ts3::AbortNotRewritten("cls_702490_LotRenderer 0x0093EA40");
    return 0;
#endif
}

int32_t Pump93EAA0(int32_t a1, int32_t a2) {
#if defined(_M_IX86)
    // 0093EAA0: null-checks 6FDE10, tests the slot-3 result directly (no esi park),
    // and tail-jumps to 573320 instead of 573300/6FC9E0/6FAC70.
    void* const obj = CallGameFunction<void*, 0x6FDE10>();
    if (obj == nullptr) {
        return 0;
    }
    void* const res = CallGameMethod<void*, 0x6FE3B0>(obj, a1, a2);
    if (res == nullptr) {
        return 0;
    }
    void* const v = CallGameVirtual<void*, 3>(res, 0x017F1D88u);
    if (v == nullptr) {
        return 0;
    }
    void* const a = CallGameMethod<void*, 0x6F4A50>(v);
    if (a == nullptr) {
        return 0;
    }
    return CallGameMethod<int32_t, 0x573320>(a);
#else
    (void)a1;
    (void)a2;
    ts3::AbortNotRewritten("cls_702490_LotRenderer 0x0093EAA0");
    return 0;
#endif
}

}  // namespace ts3::lotrenderer
