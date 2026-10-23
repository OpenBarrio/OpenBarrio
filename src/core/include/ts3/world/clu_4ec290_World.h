#pragma once
#include "ts3/layout.h"

namespace ts3::world {
struct WorldPath {
    uint16_t text[520];
    // TS3W.exe 0x004EC290: clear only the first UTF-16 code unit and return this.
    WorldPath* Clear();
    // TS3W.exe 0x004ED810: assign using conversion mode 5; always return true.
    uint8_t Assign(const uint16_t* source);
    // TS3W.exe 0x004EDA80: erase the last component, preserving a trailing separator.
    uint8_t Parent();
    // TS3W.exe 0x004EDCE0: splice using component modes 1 and 2 (ret 8).
    int Splice12(const uint16_t* source, int length);
    // TS3W.exe 0x004EDD00: splice using component modes 2 and 3 (ret 8).
    int Splice23(const uint16_t* source, int length);
};
TS3_X86_SIZE(WorldPath, 1040);
struct WorldDate {
    uint8_t prefix[196];
    int16_t hour, minute, second, day, month, year;
    // TS3W.exe 0x0059D4B0: store six shorts; AX returns month (upper EAX is unspecified).
    int16_t Set(int16_t h, int16_t m, int16_t s, int16_t d, int16_t mon, int16_t y);
};
TS3_X86_OFFSET(WorldDate, hour, 196);
TS3_X86_OFFSET(WorldDate, year, 206);
TS3_X86_SIZE(WorldDate, 208);
struct WorldScheduler {
    uint8_t prefix[240];
    void* scheduler;
    // TS3W.exe 0x00C60CB0: return the pointer at x86 offset 240.
    void* GetScheduler() const;
    // Pointer-bit adapter for the generic x86 compare ABI (no pointer normalization).
    uintptr_t GetSchedulerBits() const { return reinterpret_cast<uintptr_t>(GetScheduler()); }
};
TS3_X86_OFFSET(WorldScheduler, scheduler, 240);
TS3_X86_SIZE(WorldScheduler, 244);
struct WorldString {
    uint16_t *begin, *end, *capacity;
    const char* category;
};
TS3_X86_SIZE(WorldString, 16);
struct WorldPathOwner {
    uint8_t prefix[40];
    WorldString name;
    uint8_t middle[512];
    WorldPath path;
    // TS3W.exe 0x009C69C0: assign the path and refresh the name unless aliased.
    void* SetPath(const uint16_t* source);
};
TS3_X86_OFFSET(WorldPathOwner, name, 40);
TS3_X86_OFFSET(WorldPathOwner, path, 568);
struct WorldSerializable {
    // TS3W.exe 0x005A2620: construct a stream scope, serialize this, destroy the scope.
    uint8_t Serialize(void* stream, uint8_t mode);
};
// TS3W.exe 0x00899380: build a path from two components into the caller's string.
void* CombinePath(const uint16_t* first, const uint16_t* second, WorldString* output);
}

namespace ts3::world {
struct WorldHost {
    uint8_t prefix[68];
    uint8_t loading;
    uint8_t pad45[171];
    void* scheduler;
    uint8_t padF4[192];
    int32_t loadStarted;
    uint8_t pad1B8[184];
    uint8_t config[1];
};
TS3_X86_OFFSET(WorldHost, scheduler, 240);
TS3_X86_OFFSET(WorldHost, loadStarted, 436);
TS3_X86_OFFSET(WorldHost, config, 624);
struct WorldLoadData {
    WorldPath root;
    WorldString worldName;
    struct NarrowString { char *begin, *end, *capacity; const char* category; } userName;
    WorldPath path1, path2;
    WorldString extra;
    WorldPath path3, path4, path5;
    // TS3W.exe 0x00AAC3A0: initialize six paths and split source at the first colon.
    WorldLoadData* Init(const uint16_t* source);
};
TS3_X86_OFFSET(WorldLoadData, worldName, 1040);
TS3_X86_OFFSET(WorldLoadData, userName, 1056);
TS3_X86_OFFSET(WorldLoadData, path2, 2112);
TS3_X86_OFFSET(WorldLoadData, path3, 3168);
TS3_X86_OFFSET(WorldLoadData, path4, 4208);
TS3_X86_OFFSET(WorldLoadData, path5, 5248);
TS3_X86_SIZE(WorldLoadData, 6288);
struct WorldManagerFields {
    uint8_t prefix[256];
    const uint16_t* passportRoot;
    uint8_t middle[1376];
    void* secondary;
};
TS3_X86_OFFSET(WorldManagerFields, passportRoot, 256);
TS3_X86_OFFSET(WorldManagerFields, secondary, 1636);
TS3_X86_SIZE(WorldManagerFields, 1640);
struct WorldSaveManager {
    uint8_t prefix[24];
    void* request;
    uint8_t special, busy;
    uint8_t pad1E[14];
    void* database;
    uint8_t pad30[4];
    uint8_t *data1Begin, *data1End;
    uint8_t pad3C[8];
    uint8_t *data2Begin, *data2End;
    // TS3W.exe 0x00AAB6A0: check the save-directory metadata and legacy fallback.
    uint8_t IsLegacy(const uint16_t* name);
    // TS3W.exe 0x00AAB820: open metadata and stamp the returned record with file time.
    uint8_t ReadMetadata(const WorldLoadData* paths, WorldDate** output);
    // TS3W.exe 0x00AAD5B0: open metadata, serialize destination, and release database.
    uint8_t ReadInto(const void* input, WorldSerializable* destination);
    // TS3W.exe 0x00AB05F0: write challenge data with backup/rollback and original error codes.
    int SaveChallenge(const uint16_t* data, int count, const uint16_t* identifier);
    // TS3W.exe 0x00AB14B0: load a save, retaining EA's early-exit loading-flag leak.
    uint8_t Load(const void* input);
};
TS3_X86_OFFSET(WorldSaveManager, request, 24);
TS3_X86_OFFSET(WorldSaveManager, special, 28);
TS3_X86_OFFSET(WorldSaveManager, database, 44);
TS3_X86_OFFSET(WorldSaveManager, data1Begin, 52);
TS3_X86_OFFSET(WorldSaveManager, data2Begin, 68);
// TS3W.exe 0x007D9BA0: read thumbnail version, trying Version: then echo.
uint8_t ReadThumbnailVersion(int output);
// TS3W.exe 0x00795450: ask script resource system for the world-name key.
int GetWorldNameKey();
// TS3W.exe 0x00AAE860: construct the default/current save name and append its ID.
WorldString* GetSaveName(WorldString* output);
// TS3W.exe 0x00742BB0: resolve script strings and forward challenge data to save manager.
int LoadChallengeData(int data, int count, int identifier);
// TS3W.exe 0x00742CC0: public thunk for LoadChallengeData.
int LoadChallengeDataThunk(int data, int count, int identifier);
}
