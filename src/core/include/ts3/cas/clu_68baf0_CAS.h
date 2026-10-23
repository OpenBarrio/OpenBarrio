#pragma once

#include <cstdint>

namespace ts3::cas {

// TS3W.exe 0x0068BAF0: __thiscall (this, x, y, z) -> void; ret 0x0c.
// Bounds storage is two four-float vectors: minimum at +0, maximum at +0x10.
struct Bounds4 {
    float minimum[4];
    float maximum[4];

    void Include(float x, float y, float z);
};

// The first two slots match the game's AddRef / Release interface methods.
struct IReference {
    virtual int32_t AddRef() = 0;
    virtual int32_t Release() = 0;
};

// TS3W.exe 0x0068ED00: pointer field at +0x64; thiscall, one stack argument.
struct ReferenceHolder {
    uint8_t reserved[0x64];
    IReference* value;

    void SetReference(IReference* replacement);
};

}  // namespace ts3::cas
