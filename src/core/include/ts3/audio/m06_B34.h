#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::audio {
#pragma pack(push,4)
struct B34Packet {
    double position, zero, rate;
    const char* name;
    void* context;
    uint32_t source, word;
    float gain;
};
#pragma pack(pop)
struct B34Emitter {
    void** vtable;
    uint8_t prefix[0x370-4];
    void* source;
    uint32_t word374;
    void* metadata;
    uint8_t streaming, pad37d[3];
    void* voices;
    uint32_t timestamp, key[4];
    uint8_t pad398[0x10];
    uint32_t handle;
    uint8_t pad3ac[4];
    uint32_t word3b0;
    uint8_t pad3b4[4];
    double position;
    uint32_t word3c0;
    int32_t mode;
    uint8_t enabled, dirty, pad3ca[2];
    float pitch, volume, scale;
    uint8_t spatial[0x18];
    float attenuation, effectiveVolume, distance, rate;
    uint8_t pad400[0xc];
    void* player;
    void* node410;
    void* node414;
    void* node418;
    uint32_t word41c;
    void* fader;
    uint32_t flags, changed;
    char name[0x80];
    uint32_t pad4ac;
    B34Packet packet;
    float queryGain;
    uint8_t pad4e0[8];
    void* context;
    // TS3W.exe 0x008A1760: Read player property one as double and round to float under the caller's CW.
    float ReadPosition();
    // TS3W.exe 0x008A17C0: Dispatch two parameter keys, returning true in AL with the preceding EAX upper bytes.
    uint32_t SetParameter(uint32_t key, void* value, uint32_t unused);
    // TS3W.exe 0x008A1820: Return the borrowed voice container (near clone of DDFRecordIndex).
    void* GetVoices();
    // TS3W.exe 0x008A1830: Retain then publish a non-null voice container before releasing the old one.
    bool SetVoices(void* value);
    // TS3W.exe 0x008A1870: Retain and replace the optional source, publishing before old-source release.
    void SetSource(void* value);
    // TS3W.exe 0x008A18B0: Tick the secondary emitter interface; stop and notify according to player and pause flags.
    void Tick(uint32_t unused, float delta);
    // TS3W.exe 0x008A19F0: Apply dirty emitter properties; only the low byte of force is significant.
    void Update(uint32_t unused, uint32_t force);
    // TS3W.exe 0x008A1B90: Submit packet property five when a player exists.
    bool SubmitPacket();
    // TS3W.exe 0x008A1BE0: Return the raw voice-container word for key 0x536e5031 in EAX.
    uint32_t VoiceScalar();
    // TS3W.exe 0x008A1C00: Release the container and clear player links while holding the RW lock.
    bool Reset();
    // TS3W.exe 0x008A1C80: Send property one with a null payload to the first voice node; no null checks.
    bool ActivateFirst();
    // TS3W.exe 0x008A1CA0: Resolve and apply mode-dependent subscription parameters, then publish mode.
    void SetMode(int32_t value);
    // TS3W.exe 0x008A1E40: Build the streaming name and packet, then submit unless flag 0x800 is set.
    bool PreparePacket();
};
// TS3W.exe 0x008A1D70: Format a streamable resource name; original takes key in EDI, size in ESI and buffer on stack.
uintptr_t B34FormatName(const uint32_t* key, int32_t size, char* buffer);
TS3_X86_OFFSET(B34Emitter,source,0x370);
TS3_X86_OFFSET(B34Emitter,voices,0x380);
TS3_X86_OFFSET(B34Emitter,position,0x3b8);
TS3_X86_OFFSET(B34Emitter,flags,0x424);
TS3_X86_OFFSET(B34Emitter,packet,0x4b0);
TS3_X86_OFFSET(B34Emitter,context,0x4e8);
}
