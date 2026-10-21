// Codec objects the resource system (de)compresses package records with. On x86 the
// vtable order matches the game's (its RefPack codec: constructor 0x004EBFD0,
// vtable 0x00FB9018).
#pragma once

#include <cstdint>

#include "ts3/allocator/core_allocator.h"
#include "ts3/layout.h"

namespace ts3::resource {

// Index entry compression ids.
inline constexpr uint16_t kCompressionNone = 0x0000;
inline constexpr uint16_t kCompressionRefPack = 0xFFFF;

class ICompressionCodec {
public:
    virtual ~ICompressionCodec() = default;
    // Signature inferred from the game's `ret 0x14`; only the write path uses it.
    virtual uint32_t Compress(const void* src, uint32_t srcLen, void* dst, uint32_t dstCap, uint32_t flags) = 0;
    // dst == nullptr asks for the decompressed size.
    virtual uint32_t Decompress(const void* src, uint32_t srcLen, void* dst, uint32_t dstCap, uint32_t flags) = 0;
    virtual bool IsCompressed(const void* src, uint32_t srcLen) = 0;
    virtual const char* GetName() const = 0;
};

class RefPackCodec final : public ICompressionCodec {
public:
    explicit RefPackCodec(allocator::ICoreAllocator* allocator) : allocator_(allocator) {}

    uint32_t Compress(const void* src, uint32_t srcLen, void* dst, uint32_t dstCap, uint32_t flags) override;
    uint32_t Decompress(const void* src, uint32_t srcLen, void* dst, uint32_t dstCap, uint32_t flags) override;
    bool IsCompressed(const void* src, uint32_t srcLen) override;
    const char* GetName() const override { return "Refpack"; }

private:
    allocator::ICoreAllocator* allocator_;
};
TS3_X86_SIZE(RefPackCodec, 8);

// The object a package is attached to (DatabasePackedFile vtable slot 18 attaches it and
// notifies it through slot 13), presumably the resource manager. Only the slot the read
// path calls is recovered; the placeholders keep it at slot 27 (+0x6C).
class IResourceManager {
public:
#define TS3_UNRECOVERED(n) virtual void Unrecovered##n() = 0;
    TS3_UNRECOVERED(0) TS3_UNRECOVERED(1) TS3_UNRECOVERED(2) TS3_UNRECOVERED(3) TS3_UNRECOVERED(4)
    TS3_UNRECOVERED(5) TS3_UNRECOVERED(6) TS3_UNRECOVERED(7) TS3_UNRECOVERED(8) TS3_UNRECOVERED(9)
    TS3_UNRECOVERED(10) TS3_UNRECOVERED(11) TS3_UNRECOVERED(12) TS3_UNRECOVERED(13) TS3_UNRECOVERED(14)
    TS3_UNRECOVERED(15) TS3_UNRECOVERED(16) TS3_UNRECOVERED(17) TS3_UNRECOVERED(18) TS3_UNRECOVERED(19)
    TS3_UNRECOVERED(20) TS3_UNRECOVERED(21) TS3_UNRECOVERED(22) TS3_UNRECOVERED(23) TS3_UNRECOVERED(24)
    TS3_UNRECOVERED(25) TS3_UNRECOVERED(26)
#undef TS3_UNRECOVERED
    // Codec for a compression id other than RefPack, or nullptr. Not owned by the caller.
    virtual ICompressionCodec* GetCodec(uint32_t compression) = 0;
};

// Record decompression, which each database class compiles as its own identical copy:
// DatabasePackedFile 0x004A7360, DatabasePackedStream 0x0072C980, MemoryDB 0x0072AFB0.
// The low 16 bits of `compression` select RefPack (0xFFFF): a codec is made for the call,
// allocated under `refPackTag`. Any other value goes to the resource manager unchanged.
// Checks: IsCompressed, then the decompressed size must equal memSize, then the decode
// must not fail.
bool DecompressRecord(allocator::ICoreAllocator* allocator, IResourceManager* resourceManager, const char* refPackTag,
                      uint32_t compression, const void* src, uint32_t srcLen, uint32_t memSize, void* dst);

}  // namespace ts3::resource
