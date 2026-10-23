// Unit clu_a1a290_ResourceSystem (the project notes): the six constructors of the SIGS
// (SocialFeaturesManager) sub-objects. All six are __thiscall, take only `this`, return
// `this`, and every field write is an unconditional constant, so an object's post-ctor
// bytes are the same whatever memory it starts from. The callers name the family:
// SocialFeaturesManager::Init (0x00A3FF00) news the 0x140- and 0x38-byte objects, and the
// "SIGS/Crossroads/ProfileManager/AddOnlineProfile" code (0x00A3FBC0) news and caches the
// 0x2B8-byte one (hence ProfileStore; the other names are structural, EA's are unknown).
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::resourcesystem {

// The lock object TS3W.exe's constructor 0x004E17E0 builds (unit clu_4e17e0_ResourceSystem,
// not rewritten yet: every Construct below starts its locks through the bridge). 0x2C
// bytes: the CRITICAL_SECTION at +0x00 (its DebugInfo is a fresh heap pointer per
// InitializeCriticalSection call), then the slots at +0x20/+0x24/+0x28 the ctor sets for
// the name and the section flag. Replace this struct when clu_4e17e0 is rewritten.
struct GameLock {
    uint8_t bytes[0x2C];
};
TS3_X86_SIZE(GameLock, 0x2C);

// The 0x84-byte object the game's 0x00A43680 builds inside ProfileRooms (+0x20). The
// function belongs to no unit in the project notes, so ProfileRooms::Construct calls it
// through the bridge; only what its caller touches afterwards is named here.
struct DepObjectA43680 {
    uint8_t head[0x10];  // +0x00: ProfileRooms zeroes the first dword again after the ctor
    void* released;      // +0x10: the ref-counted pointer ProfileRooms::Construct drops
                         // (object +0x30)
    uint8_t tail[0x70];  // +0x14..+0x84: the dependency's own business
};
TS3_X86_OFFSET(DepObjectA43680, released, 0x10);
TS3_X86_SIZE(DepObjectA43680, 0x84);

// 0x00A3FAE0, 0x38 bytes: two cleared dwords and one lock; allocated by
// SocialFeaturesManager::Init into its +0x14 slot.
struct SigsServiceLock {
    SigsServiceLock* Construct();  // the original 0x00A3FAE0

    uint32_t field00;  // 0x00
    uint32_t field04;  // 0x04
    GameLock lock;     // 0x08
    uint8_t tail[4];   // 0x34: never written by the ctor
};
TS3_X86_OFFSET(SigsServiceLock, lock, 0x08);
TS3_X86_SIZE(SigsServiceLock, 0x38);

// 0x00A1A290, 0x140 bytes: SocialFeaturesManager::Init's first allocation (its +0x00).
// Four locks; note fieldE8 starts at 2, not 0.
struct SigsServices {
    SigsServices* Construct();  // the original 0x00A1A290

    uint32_t field00;       // 0x00
    uint8_t gap04[4];       // 0x04: not written
    uint32_t field08;       // 0x08
    uint32_t field0C;       // 0x0C
    uint32_t field10;       // 0x10
    uint32_t field14;       // 0x14
    uint32_t field18;       // 0x18
    uint32_t field1C;       // 0x1C
    uint32_t field20;       // 0x20
    uint8_t gap24[4];       // 0x24: not written
    GameLock lockA;         // 0x28
    uint8_t gap54[4];       // 0x54: not written
    uint32_t field58;       // 0x58
    uint32_t field5C;       // 0x5C
    uint32_t field60;       // 0x60
    uint32_t field64;       // 0x64
    uint32_t field68;       // 0x68
    uint32_t field6C;       // 0x6C
    uint32_t field70;       // 0x70
    uint8_t gap74[4];       // 0x74: not written
    GameLock lockB;         // 0x78
    uint8_t gapA4[4];       // 0xA4: not written
    GameLock lockC;         // 0xA8
    uint8_t gapD4[4];       // 0xD4: not written
    uint32_t fieldD8;       // 0xD8
    uint32_t fieldDC;       // 0xDC
    uint8_t fieldE0;        // 0xE0 (byte)
    uint8_t gapE1[3];       // 0xE1: not written
    uint32_t fieldE4;       // 0xE4
    uint32_t fieldE8;       // 0xE8: starts at 2
    uint32_t fieldEC;       // 0xEC
    uint32_t fieldF0;       // 0xF0
    uint8_t gapF4[4];       // 0xF4: not written
    uint32_t fieldF8;       // 0xF8
    uint32_t fieldFC;       // 0xFC
    uint32_t field100;      // 0x100
    uint32_t field104;      // 0x104
    GameLock lockD;         // 0x108
    uint8_t tail[0xC];      // 0x134: never written by the ctor
};
TS3_X86_OFFSET(SigsServices, lockA, 0x28);
TS3_X86_OFFSET(SigsServices, lockB, 0x78);
TS3_X86_OFFSET(SigsServices, lockC, 0xA8);
TS3_X86_OFFSET(SigsServices, fieldE8, 0xE8);
TS3_X86_OFFSET(SigsServices, lockD, 0x108);
TS3_X86_SIZE(SigsServices, 0x140);

// 0x00A35DF0, 0x70 bytes: the SigsProfileStore's +0xF0 part. One lock at +0x40; its
// +0x3C is zeroed here and overwritten with the store's own pointer by the store's ctor.
struct ProfileConnections {
    ProfileConnections* Construct();  // the original 0x00A35DF0

    uint32_t field00;       // 0x00
    uint32_t field04;       // 0x04
    uint32_t field08;       // 0x08
    uint32_t field0C;       // 0x0C
    uint32_t field10;       // 0x10
    uint32_t field14;       // 0x14
    uint32_t field18;       // 0x18
    uint32_t field1C;       // 0x1C
    uint32_t field20;       // 0x20
    uint32_t field24;       // 0x24
    uint32_t field28;       // 0x28
    uint8_t bytes2C[4];     // 0x2C
    uint32_t field30;       // 0x30
    uint32_t field34;       // 0x34
    uint32_t field38;       // 0x38
    void* owner;            // 0x3C: 0 here, the store's this in SigsProfileStore::Construct
    GameLock lock;          // 0x40
    uint8_t tail[4];        // 0x6C: never written by the ctor
};
TS3_X86_OFFSET(ProfileConnections, owner, 0x3C);
TS3_X86_OFFSET(ProfileConnections, lock, 0x40);
TS3_X86_SIZE(ProfileConnections, 0x70);

// 0x00A43970, 0xE0 bytes: the SigsProfileStore's +0x160 part. Builds the dependency
// object at +0x20 through the bridge (0x00A43680), then a lock at +0xB0.
struct ProfileRooms {
    ProfileRooms* Construct();  // the original 0x00A43970

    uint32_t field00;       // 0x00
    uint32_t field04;       // 0x04
    uint32_t field08;       // 0x08
    uint32_t field0C;       // 0x0C
    uint32_t field10;       // 0x10
    uint32_t field14;       // 0x14
    uint32_t field18;       // 0x18
    uint8_t bytes1C[3];     // 0x1C
    uint8_t gap1F;          // 0x1F: not written
    DepObjectA43680 embedded;  // 0x20, built by the game's 0x00A43680
    uint8_t gapA4[4];       // 0xA4: not written
    void* owner;            // 0xA8: 0 here, the store's this in SigsProfileStore::Construct
    uint32_t fieldAC;       // 0xAC: 15
    GameLock lock;          // 0xB0
    uint8_t tail[4];        // 0xDC: never written by the ctor
};
TS3_X86_OFFSET(ProfileRooms, embedded, 0x20);
TS3_X86_OFFSET(ProfileRooms, owner, 0xA8);
TS3_X86_OFFSET(ProfileRooms, lock, 0xB0);
TS3_X86_SIZE(ProfileRooms, 0xE0);

// 0x00A400A0, 0x78 bytes: the SigsProfileStore's +0x240 part. One lock at +0x20; the
// only non-zero field is the limit 31 at +0x14.
struct ProfileQueue {
    ProfileQueue* Construct();  // the original 0x00A400A0

    uint32_t field00;    // 0x00
    uint32_t field04;    // 0x04
    uint32_t field08;    // 0x08
    uint32_t field0C;    // 0x0C
    uint8_t bytes10[2];  // 0x10
    uint8_t gap12[2];    // 0x12: not written
    uint32_t limit;      // 0x14: 31
    void* owner;         // 0x18: 0 here, the store's this in SigsProfileStore::Construct
    uint8_t gap1C[4];    // 0x1C: not written
    GameLock lock;       // 0x20
    uint8_t tail[0x2C];  // 0x4C: not written here; the store's ctor zeroes tail[0x28..0x2A]
};
TS3_X86_OFFSET(ProfileQueue, limit, 0x14);
TS3_X86_OFFSET(ProfileQueue, owner, 0x18);
TS3_X86_OFFSET(ProfileQueue, lock, 0x20);
TS3_X86_SIZE(ProfileQueue, 0x78);

// 0x00A3B540, 0x2B8 bytes: the object the AddOnlineProfile path news (0x2B8 bytes) and
// caches per profile-manager. Two zeroing memsets (0x00..0x30 and 0x30..0x9C), the three
// sub-constructors, three dead releases (the released slots are zeroed a few instructions
// earlier, so the ref-counted branch can never fire during construction — kept as the
// original has it) and three self-pointers into the sub-objects.
struct SigsProfileStore {
    SigsProfileStore* Construct();  // the original 0x00A3B540

    uint8_t memset00[0x24];  // 0x00: cleared by the first memset
    uint32_t field24;        // 0x24: -1 (after the memsets)
    uint8_t memset28[0x30];  // 0x28: cleared by the second memset
    void* field58;           // 0x58: released slot (0)
    uint8_t memset5C[0x44];  // 0x5C: cleared by the second memset
    uint32_t limitA0;        // 0xA0: 0x0FFFFFFF
    uint32_t fieldA4;        // 0xA4: 0
    uint32_t limitA8;        // 0xA8: 0x0FFFFFFF
    uint32_t fieldAC;        // 0xAC: 0
    uint32_t fieldB0;        // 0xB0
    uint32_t fieldB4;        // 0xB4
    uint32_t fieldB8;        // 0xB8
    uint32_t fieldBC;        // 0xBC
    uint32_t fieldC0;        // 0xC0
    uint32_t fieldC4;        // 0xC4
    uint32_t fieldC8;        // 0xC8: -1
    uint32_t fieldCC;        // 0xCC
    uint32_t fieldD0;        // 0xD0
    uint32_t fieldD4;        // 0xD4
    uint32_t fieldD8;        // 0xD8
    uint32_t fieldDC;        // 0xDC
    uint32_t fieldE0;        // 0xE0
    void* fieldE4;           // 0xE4: released slot (0)
    void* fieldE8;           // 0xE8: released slot (0)
    uint8_t gapEC[4];        // 0xEC: not written
    ProfileConnections connections;  // 0xF0
    ProfileRooms rooms;              // 0x160
    ProfileQueue queue;              // 0x240
};
TS3_X86_OFFSET(SigsProfileStore, field24, 0x24);
TS3_X86_OFFSET(SigsProfileStore, field58, 0x58);
TS3_X86_OFFSET(SigsProfileStore, limitA0, 0xA0);
TS3_X86_OFFSET(SigsProfileStore, limitA8, 0xA8);
TS3_X86_OFFSET(SigsProfileStore, fieldC8, 0xC8);
TS3_X86_OFFSET(SigsProfileStore, fieldE8, 0xE8);
TS3_X86_OFFSET(SigsProfileStore, connections, 0xF0);
TS3_X86_OFFSET(SigsProfileStore, rooms, 0x160);
TS3_X86_OFFSET(SigsProfileStore, queue, 0x240);
TS3_X86_SIZE(SigsProfileStore, 0x2B8);

}  // namespace ts3::resourcesystem
