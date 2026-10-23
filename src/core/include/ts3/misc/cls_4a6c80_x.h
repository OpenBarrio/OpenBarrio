#pragma once
#include <cstdint>
#include "ts3/resource/packed_record.h"
#include "ts3/resource/packed_file.h"
#include "ts3/resource/package_header.h"

namespace ts3::misc {
// The game's IStream subobject at PackedFileRecord + 0x28. The owner layout is
// shared with the verified packed-record implementation (including pointer growth).
struct RecordReadStream {
    // TS3W.exe 0x004AD670: returns the read-record stream type identifier.
    uint32_t Type();
    // TS3W.exe 0x004AD680: returns region size or forwards to the embedded stream.
    uint32_t Size();
    // TS3W.exe 0x004AD6A0: returns the position; memory forwarding always uses origin zero.
    int32_t Position(int32_t origin);
    // TS3W.exe 0x004AD6E0: seeks a region or embedded stream; end-relative underflow is kept.
    bool Seek(int32_t distance, int32_t origin);
    // TS3W.exe 0x004AD790: returns unsigned remaining size, without clamping underflow.
    uint32_t Available();
    // TS3W.exe 0x004AD7B0: reads a region or embedded stream; closed region returns UINT_MAX.
    uint32_t Read(void* dst, uint32_t count);
    // TS3W.exe 0x004ADEF0: adjusts the stream pointer and invokes the deleting destructor.
    void* Delete(uint32_t flags);
    // TS3W.exe 0x004ADF00: adjusts this and closes through the owner.
    bool Close();
    // TS3W.exe 0x004ADF10: adjusts this and forwards the reference release.
    int32_t Release();
    // TS3W.exe 0x004ADF20: adjusts this and forwards the reference add.
    int32_t AddRef();
    // TS3W.exe 0x0093E770: returns the containing record's access flags.
    uint32_t Access();
private:
    resource::PackedFileRecord* Owner();
};
struct ReadRecordDeleter {
    // TS3W.exe 0x004AE420: destructs the read record and optionally frees its allocation.
    void* Delete(uint32_t flags);
};
struct RecordKeyView {
    void* const* vtable;
    uint32_t prefix[3];
    uint32_t key[4];
    // TS3W.exe 0x00580430: returns the record's four-word key subobject.
    uint32_t* Key();
    // TS3W.exe 0x00739A60: copies four key words in order, returning the fourth word.
    uint32_t SetKey(const void* source);
};
TS3_X86_OFFSET(RecordKeyView, key, 0x10);
TS3_X86_SIZE(RecordKeyView, 0x20);
struct alignas(16) LightVector { float lane[4]; };
struct RectangleLightEntries {
    void* const* vtable;
    uint8_t unknown004[12];
    LightVector tint;
    uint8_t unknown020[16];
    LightVector basis[4];
    uint8_t unknown070[0x40];
    uint32_t context;
    uint8_t unknown0b4[0x2c];
    LightVector activeColor, scale;
    uint8_t flags;
    uint8_t unknown101[15];
    LightVector localPosition, worldPosition;
    uint8_t unknown130[0x20];
    LightVector attenuation;
    LightVector localA, localB, localColor, worldA, worldB, normal;
    LightVector multiplier, direction, highlight, diffuse;
    uint8_t frontFacing;
    uint8_t unknown201[7];
    float area;
    uint8_t unknown20c[4];
    LightVector bounds;
    float extent;
    // TS3W.exe 0x006BCD70: transforms position/basis vectors, computes area, invokes slot 14.
    int32_t UpdateTransform();
    // TS3W.exe 0x006BF2A0: flips two local basis vectors and lowers Z, then invokes slot 23.
    uint8_t Flip(uint32_t enabled);
    // TS3W.exe 0x006BCB00: returns the intensity's float-average times a property through ST0.
    double Intensity();
    // TS3W.exe 0x006BF320: refreshes view direction and colors from the lighting environment.
    bool RefreshEnvironment();
    // TS3W.exe 0x006BF540: clips a projected ray against the window, emitting color and direction.
    LightVector* Intersect(LightVector* color, const LightVector* ray, LightVector* directionOut, float* weight);
    // TS3W.exe 0x006BF780: computes distance and angular attenuation with EA reciprocal refinement.
    LightVector* Attenuate(void* output, const void* input);
    // TS3W.exe 0x006BF880: evaluates the light at a position using the supplied angular frame.
    uintptr_t Evaluate(const LightVector* position, const LightVector* frame, LightVector* out);
    // TS3W.exe 0x006C0E60: invokes the base initializer and installs rectangular-light parameters.
    RectangleLightEntries* Construct(const float* parameters, void* contextArg);
    // TS3W.exe 0x006C0FE0: computes blended sample directions/colors and submits to a sample object.
    void* Sample(const LightVector* position, void* output);
};
struct PackedIndexWriter : resource::DatabasePackedFile {
    // TS3W.exe 0x004A8680: serializes and writes a changed index, freeing temporary bytes on all paths.
    bool WriteIndex();
    // TS3W.exe 0x009EED90: slot 38's void index destructor; null is a no-op (EAX is undefined).
    void DestroyIndex(void* object);
    // TS3W.exe 0x004A6C80: ABI adapter to the already-verified mutable-index implementation.
    void MakeMutable();
    // TS3W.exe 0x004A7AD0: validates the header against the locked file size, preserving the offset override.
    bool HeaderValid(const resource::PackageHeader* header);
    // TS3W.exe 0x004A7D80: ABI adapter to the verified index validator.
    bool CheckIndex();
    // TS3W.exe 0x004A8810: ABI adapter to the verified free-space rebuilder.
    void RebuildSpace();
    // TS3W.exe 0x004A9950: ABI adapter to the verified index loader.
    bool ReadIndex();
};
TS3_X86_OFFSET(RectangleLightEntries, tint, 0x10);
TS3_X86_OFFSET(RectangleLightEntries, context, 0xb0);
TS3_X86_OFFSET(RectangleLightEntries, multiplier, 0x1c0);
TS3_X86_OFFSET(RectangleLightEntries, diffuse, 0x1f0);
TS3_X86_OFFSET(RectangleLightEntries, frontFacing, 0x200);
TS3_X86_OFFSET(RectangleLightEntries, bounds, 0x210);
TS3_X86_OFFSET(RectangleLightEntries, basis, 0x30);
TS3_X86_OFFSET(RectangleLightEntries, flags, 0x100);
TS3_X86_OFFSET(RectangleLightEntries, localA, 0x160);
TS3_X86_OFFSET(RectangleLightEntries, normal, 0x1b0);
TS3_X86_OFFSET(RectangleLightEntries, area, 0x208);
TS3_X86_OFFSET(RectangleLightEntries, extent, 0x220);
TS3_X86_SIZE(RectangleLightEntries, 0x230);
struct AnimationComponentEntries {
    void* const* vtable;
    void* owner;
    void* const* secondary[3];
    uint32_t unknown14, unknown18, unknown1c;
    void* listener;
    void* runtime;
    void* binding;
    uint32_t flags, unknown30, unknown34;
    uint8_t status;
    // TS3W.exe 0x0093E5C0: initializes the animation component and conditionally registers its owner.
    AnimationComponentEntries* Construct(void* newOwner);
    // TS3W.exe 0x0093F580: lazily allocates a runtime and seeds it from an owner's binding.
    bool EnsureRuntime();
    // TS3W.exe 0x00941CD0: destroys component state, releases its listener, restores base vtables.
    int32_t Cleanup();
};
TS3_X86_OFFSET(AnimationComponentEntries, owner, 0x04);
TS3_X86_OFFSET(AnimationComponentEntries, secondary, 0x08);
TS3_X86_OFFSET(AnimationComponentEntries, runtime, 0x24);
TS3_X86_OFFSET(AnimationComponentEntries, flags, 0x2c);
TS3_X86_OFFSET(AnimationComponentEntries, status, 0x38);
TS3_X86_SIZE(AnimationComponentEntries, 0x3c);
} // namespace ts3::misc
