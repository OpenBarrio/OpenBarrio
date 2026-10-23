#include "ts3/world/clu_4ec290_World.h"
#include <cstring>
#include "ts3/world/clu_4dbc30_World.h"
#include <cstdio>
#include <ctime>
#include "ts3/world/clu_4ec520_World.h"
#include "ts3/resourcesystem/clu_455970_ResourceSystem.h"

namespace ts3::world {
WorldPath* WorldPath::Clear() // 0x004EC290
{
    text[0] = 0;
    return this;
}
uint8_t WorldPath::Assign(const uint16_t* source) // 0x004ED810
{
    int scratch = 0;
    resourcesystem::NormalizePath(reinterpret_cast<char16_t*>(text), reinterpret_cast<const char16_t*>(source),
                                  reinterpret_cast<const char16_t*>(&scratch), 5);
    return 1;
}
uint8_t WorldPath::Parent() // 0x004EDA80
{
    uint16_t *first, *last;
    const wchar_t *base, *file, *extension;
    SplitPath(reinterpret_cast<const wchar_t*>(text), &base, &file, &extension);
    first = const_cast<uint16_t*>(reinterpret_cast<const uint16_t*>(base));
    last = const_cast<uint16_t*>(reinterpret_cast<const uint16_t*>(file));
    if (first == last) return 0;
    auto p = last;
    if (p[-1] == '\\') --p;
    if (p <= first) return 0;
    do {
        --p;
        if (*p == '\\') {
            uint16_t empty = 0;
            return CallGameMethod<uint8_t, 0x004EC2A0>(this, p + 1, last, &empty, 0, 0, 0);
        }
    } while (p > first);
    return 0;
}
int WorldPath::Splice12(const uint16_t* source, int length) // 0x004EDCE0
{
    return CallGameMethod<int, 0x004EDB50>(this, 1, 2, source, length);
}
int WorldPath::Splice23(const uint16_t* source, int length) // 0x004EDD00
{
    return CallGameMethod<int, 0x004EDB50>(this, 2, 3, source, length);
}
int16_t WorldDate::Set(int16_t h, int16_t m, int16_t s, int16_t d, int16_t mon, int16_t y) // 0x0059D4B0
{
    hour = h;
    minute = m;
    second = s;
    day = d;
    month = mon;
    year = y;
    return mon;
}
void* WorldScheduler::GetScheduler() const // 0x00C60CB0
{
    return scheduler;
}
uint8_t WorldSerializable::Serialize(void* stream, uint8_t mode) // 0x005A2620
{
    // Opaque stack scope: six x86 pointer-width slots, never narrowed on x64.
    uintptr_t scope[6];
    CallGameMethod<void, 0x007D76F0>(scope, stream, 1);
    const auto result = CallGameMethod<uint8_t, 0x005A22F0>(this, scope, mode);
    CallGameMethod<void, 0x007D7760>(scope);
    return result;
}
void* CombinePath(const uint16_t* first, const uint16_t* second, WorldString* output) // 0x00899380
{
    WorldPath path;
    path.Clear();
    path.Splice12(first, 0);
    path.Splice23(second, 0);
    auto begin = CallGameMethod<uint16_t*, 0x00F626F0>(&path);
    auto end = begin;
    while (*end) ++end;
    return CallGameMethod<void*, 0x00408E20>(output, begin, end);
}
void* WorldPathOwner::SetPath(const uint16_t* source) // 0x009C69C0
{
    path.Assign(source);
    auto result = CallGameMethod<uint16_t*, 0x004ED5C0>(&path);
    if (name.begin == result) return result;
    if (name.begin != name.end) {
        *name.begin = 0;
        name.end = name.begin;
    }
    auto end = result;
    while (*end) ++end;
    return CallGameMethod<void*, 0x0083B590>(&name, result, end);
}
}

namespace ts3::world {
namespace {
template<class T, unsigned Address> T* GameData()
{
#if defined(_M_IX86)
    return reinterpret_cast<T*>(static_cast<uintptr_t>(Address));
#else
    AbortNotRewritten("world game data");
    return nullptr;
#endif
}
const uint16_t* End(const uint16_t* p) { auto q = p; while (*q) ++q; return q; }
WorldString EmptyString(const char* category = GameData<char, 0x010015FC>())
{
    auto p = GameData<uint16_t, 0x011C7E54>();
    return {p, p, p + 1, category};
}
void FreeString(WorldString& s, bool wide = true)
{
    auto span = reinterpret_cast<uintptr_t>(s.capacity) - reinterpret_cast<uintptr_t>(s.begin);
    if (wide) span &= ~uintptr_t(1);
    if (static_cast<intptr_t>(span) > (wide ? 2 : 1) && s.begin)
        CallGameFunction<void, 0x004E3F70>(s.begin);
}
uint16_t* Text(WorldPath& path) { return CallGameMethod<uint16_t*, 0x00F626F0>(&path); }
void AssignString(WorldString& s, const uint16_t* first, const uint16_t* last)
{ CallGameMethod<void, 0x00408E20>(&s, first, last); }
void Release(void* p)
{
    auto ref = static_cast<uint8_t*>(p) + sizeof(void*);
    CallGameVirtual<void, 1>(ref);
}
void SetDatabase(void*& current, void* value)
{
    if (current == value) return;
    if (value) CallGameVirtual<void, 0>(static_cast<uint8_t*>(value) + sizeof(void*));
    auto old = current;
    current = value;
    if (old) Release(old);
}
void* Allocate(unsigned size, const char* tag)
{ return CallGameFunction<void*, 0x004E41F0>(size, tag, 0, 0, 0, 0); }
}
uint8_t ReadThumbnailVersion(int output) // 0x007D9BA0
{
    WorldPath path;
    path.Clear();
    auto global = GameData<uint8_t, 0x011CDDE0>();
    CallGameMethod<void*, 0x00594D20>(global); // Deliberate duplicate getter.
    auto source = CallGameMethod<const uint16_t**, 0x00594D20>(global);
    path.Assign(*source);
    auto result = path.Parent();
    if (!result) return result;
    CallGameMethod<void, 0x004EDB00>(&path, GameData<uint16_t, 0x0101A6D8>());
    CallGameMethod<void, 0x004EDD60>(&path, GameData<uint16_t, 0x0101A6AC>(), 0);
    result = CallGameFunction<uint8_t, 0x007D9A50>(Text(path), GameData<char, 0x0101A6A0>(), output, 1);
    if (!result)
        result = CallGameFunction<uint8_t, 0x007D9A50>(Text(path), GameData<char, 0x0101A698>(), output, 1);
    return result;
}
uint8_t WorldSaveManager::IsLegacy(const uint16_t* name) // 0x00AAB6A0
{
    WorldPath path;
    path.Clear();
    CallGameMethod<void, 0x00AAB430>(this, &path);
    CallGameMethod<void, 0x004EDB00>(&path, name);
    if (!CallGameFunction<uint8_t, 0x004DBF60>(Text(path))) return 0;
    CallGameMethod<void, 0x004EDD60>(&path, GameData<uint16_t, 0x01064658>(), 0);
    if (CallGameFunction<uint8_t, 0x004DBBA0>(Text(path))) return 0;
    CallGameMethod<void, 0x004EDD60>(&path, *GameData<uint16_t*, 0x01182DE4>(), 0);
    if (CallGameFunction<uint8_t, 0x004DBBA0>(Text(path))) return 0;
    path.Parent();
    path.Splice23(name, 0);
    CallGameMethod<void, 0x004EDD20>(&path, GameData<uint16_t, 0x01064648>(), 0);
    return CallGameFunction<uint8_t, 0x004DBBA0>(Text(path)) != 0;
}
WorldString* GetSaveName(WorldString* output) // 0x00AAE860
{
    auto host = *GameData<WorldHost*, 0x011ECBC4>();
    *output = EmptyString();
    auto notSet = GameData<uint16_t, 0x010648FC>();
    AssignString(*output, notSet, End(notSet));
    if (host) {
        WorldPath path;
        path.Clear();
        path.Assign(static_cast<const uint16_t*>(host->scheduler));
        uint16_t buffer[200];
        CallGameMethod<void, 0x004ED670>(&path, buffer, 200);
        AssignString(*output, buffer, End(buffer));
        auto id = CallGameMethod<unsigned, 0x005A2110>(host->config);
        CallGameFunction<void, 0x004AEFE0>(output, GameData<uint16_t, 0x010648EC>(), id);
    }
    return output;
}
int GetWorldNameKey() // 0x00795450
{
    auto host = *GameData<WorldHost*, 0x011ECBC4>();
    auto resources = CallGameFunction<void*, 0x007692B0>();
    if (!host || !resources) return 0;
    WorldPath path;
    path.Clear();
    path.Assign(static_cast<const uint16_t*>(host->scheduler));
    uint16_t buffer[260];
    CopyFileNamePart(reinterpret_cast<const wchar_t*>(path.text), reinterpret_cast<wchar_t*>(buffer), 260);
    auto name = EmptyString(GameData<char, 0x01010B9C>());
    AssignString(name, buffer, End(buffer));
    struct Variant {
        void* vtable;
        uintptr_t data;
        uint16_t flags, type;
        uintptr_t tail[5];
    } a, b;
    TS3_X86_SIZE(Variant, 32);
    a.vtable = GameData<void, 0x00FB96B4>();
    a.flags = 0; a.type = 0;
    CallGameMethod<void, 0x004F5800>(&a, 19, 9, &name, 16, 1);
    b.vtable = GameData<void, 0x01006FB4>();
    b.flags = 2; b.type = 19;
    CallGameMethod<void, 0x004F62C0>(&b, &a);
    a.vtable = GameData<void, 0x00FB96B4>();
    if (a.flags & 4) CallGameMethod<void, 0x004F55A0>(&a, 0);
    int result;
    CallGameVirtual<void, 20>(resources, &b, &result);
    b.vtable = GameData<void, 0x00FB96B4>();
    if (b.flags & 4) CallGameMethod<void, 0x004F55A0>(&b, 0);
    FreeString(name);
    return result;
}
}

namespace ts3::world {
WorldLoadData* WorldLoadData::Init(const uint16_t* source) // 0x00AAC3A0
{
    root.Clear();
    worldName = EmptyString();
    auto empty = reinterpret_cast<char*>(worldName.begin);
    userName = {empty, empty, empty + 1, worldName.category};
    path1.Clear(); path2.Clear();
    extra = EmptyString();
    path3.Clear(); path4.Clear(); path5.Clear();
    if (!source) return this;
    auto input = EmptyString();
    AssignString(input, source, End(source));
    const auto count = static_cast<unsigned>(input.end - input.begin);
    auto colon = input.begin;
    while (colon != input.end && *colon != ':') ++colon;
    if (colon == input.end) {
        AssignString(worldName, input.begin, input.end);
        if (userName.begin != userName.end) {
            *userName.begin = 0;
            userName.end = userName.begin;
        }
    } else {
        const auto index = static_cast<unsigned>(colon - input.begin);
        WorldString temporary{nullptr, nullptr, nullptr, input.category};
        CallGameMethod<void, 0x00408830>(&temporary, index + 1);
        std::memmove(temporary.begin, input.begin, index * sizeof(uint16_t));
        temporary.begin[index] = 0;
        AssignString(worldName, temporary.begin, temporary.begin + index);
        FreeString(temporary);
        auto suffix = EmptyString();
        auto result = CallGameMethod<WorldString*, 0x0053EC80>(&input, &temporary, index + 1, count);
        if (result != &suffix) AssignString(suffix, result->begin, result->end);
        FreeString(temporary);
        result = CallGameFunction<WorldString*, 0x004F39F0>(&temporary, &suffix);
        if (reinterpret_cast<void*>(result) != &userName)
            CallGameMethod<void, 0x00405A50>(&userName, result->begin, result->end);
        FreeString(temporary, false);
        FreeString(suffix);
    }
    FreeString(input);
    return this;
}
uint8_t WorldSaveManager::ReadInto(const void* input, WorldSerializable* destination) // 0x00AAD5B0
{
    struct Input { uintptr_t pad[2]; const uint16_t *first, *last; };
    const auto args = static_cast<const Input*>(input);
    auto name = EmptyString();
    AssignString(name, args->first, args->last);
    if (name.begin == name.end) { FreeString(name); return 0; }
    WorldPath path;
    path.Clear();
    CallGameMethod<void, 0x004EDD60>(&path, name.begin, 0);
    if (!CallGameMethod<uint8_t, 0x00AAB790>(this, &path)) { FreeString(name); return 0; }
    if (CallGameFunction<uint8_t, 0x00996590>(Text(path))) {
        auto user = EmptyString(GameData<char, 0x010647C4>());
        user.capacity = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(user.begin) + 1);
        if (!CallGameFunction<uint8_t, 0x00995690>(&user)) {
            FreeString(user, false); FreeString(name); return 0;
        }
        auto block = Allocate(936, GameData<char, 0x0106466C>());
        auto db = block ? CallGameMethod<void*, 0x00996680>(block, user.begin, Text(path), 0) : nullptr;
        CallGameMethod<void, 0x004A0A00>(&database, db);
        FreeString(user, false);
    } else {
        auto block = Allocate(176, GameData<char, 0x0106466C>());
        auto db = block ? CallGameMethod<void*, 0x0072CC60>(block, 0) : nullptr;
        CallGameMethod<void, 0x004A0A00>(&database, db);
        CallGameVirtual<void, 11>(database, Text(path));
    }
    uint8_t result = 0;
    if (CallGameVirtual<uint8_t, 1>(database)) {
        if (CallGameVirtual<uint8_t, 6>(database, 1, 3, 0)) result = destination->Serialize(database, 1);
        CallGameVirtual<void, 2>(database);
    }
    auto old = database;
    if (old) { database = nullptr; Release(old); }
    FreeString(name);
    return result;
}
}

namespace ts3::world {
uint8_t WorldSaveManager::ReadMetadata(const WorldLoadData* paths, WorldDate** output) // 0x00AAB820
{
    WorldPath path;
    path.Clear();
    path = paths->root;
    const bool rootExists = CallGameFunction<uint8_t, 0x004DBF60>(Text(path)) != 0;
    path = rootExists ? paths->path4 : paths->path3;
    const int64_t timestamp = CallGameFunction<int64_t, 0x004DBCF0>(Text(path), 2);
    std::tm* time;
#if defined(_M_IX86)
    using Localtime = std::tm*(__cdecl*)(const int64_t*);
    time = (*GameData<Localtime, 0x00F955E4>())(&timestamp);
#else
    (void)timestamp;
    AbortNotRewritten("game localtime64 import");
    time = nullptr;
#endif
    auto block = Allocate(176, GameData<char, 0x0106466C>());
    auto db = block ? CallGameMethod<void*, 0x0072CC60>(block, 0) : nullptr;
    SetDatabase(database, db);
    if (CallGameVirtual<uint8_t, 1>(database)) {
        CallGameVirtual<void, 11>(database, Text(path));
        if (CallGameVirtual<uint8_t, 6>(database, 1, 3, 0)) {
            const uint32_t key[4] = {1, 0, 0x628a788f, 0};
            WorldDate* record;
            auto factory = CallGameFunction<void*, 0x004AFD20>();
            if (CallGameVirtual<uint8_t, 3>(factory, key, &record, 0, database, 0, 0)) {
                *output = record;
                record->Set(static_cast<int16_t>(time->tm_hour), static_cast<int16_t>(time->tm_min),
                            static_cast<int16_t>(time->tm_sec), static_cast<int16_t>(time->tm_mday),
                            static_cast<int16_t>(time->tm_mon + 1), static_cast<int16_t>(time->tm_year + 1900));
                return 1;
            }
        }
        CallGameVirtual<void, 2>(database);
    }
    auto old = database;
    if (old) { database = nullptr; Release(old); }
    return 0;
}
}

namespace ts3::world {
namespace {
WorldString* Join(WorldString* out, const WorldString* a, const WorldString* b)
{
    TSString4dbc30 left{a->begin, a->end, a->capacity, a->category};
    TSString4dbc30 right{b->begin, b->end, b->capacity, b->category}, result;
    Concat7D9C50(&result, &left, &right);
    *out = {static_cast<uint16_t*>(result.begin), static_cast<uint16_t*>(result.end), static_cast<uint16_t*>(result.cap), result.allocName};
    return out;
}
WorldString* Append(WorldString* out, const WorldString* a, const uint16_t* suffix)
{
    TSString4dbc30 left{a->begin, a->end, a->capacity, a->category}, result;
    ConcatAppend7D9CC0(&result, &left, reinterpret_cast<const wchar_t*>(suffix));
    *out = {static_cast<uint16_t*>(result.begin), static_cast<uint16_t*>(result.end), static_cast<uint16_t*>(result.cap), result.allocName};
    return out;
}
// These verified stream delegates predate the portable pointer ABI. Never narrow a
// pointer on x64: the original game/writer exists only in the x86 host.
void StreamInt(uintptr_t* scope, int value)
{ reinterpret_cast<LoadSaveWrapper5FE3A0*>(scope)->Invoke(value); }
void StreamHash(uintptr_t* scope, uint64_t hash)
{ reinterpret_cast<LoadSaveWrapper56CD30*>(scope)->Invoke(static_cast<int32_t>(hash), static_cast<int32_t>(hash >> 32)); }
void StreamBytes(uintptr_t* scope, const void* data, int count)
{
#if defined(_M_IX86)
    reinterpret_cast<LoadSaveWrapper59F5B0*>(scope)->Invoke(static_cast<int32_t>(reinterpret_cast<uintptr_t>(data)), count);
#else
    (void)scope; (void)data; (void)count;
    AbortNotRewritten("verified stream delegate has an x86-only pointer ABI");
#endif
}
int ByteSpan(const uint8_t* first, const uint8_t* last)
{ return static_cast<int32_t>(reinterpret_cast<uintptr_t>(last) - reinterpret_cast<uintptr_t>(first)); }
void StreamRaw(uintptr_t* scope, const void* data, int count)
{
#if defined(_M_IX86)
    reinterpret_cast<LoadSaveWriter7D84E0*>(scope)->Write(static_cast<int32_t>(reinterpret_cast<uintptr_t>(data)), count);
#else
    (void)scope; (void)data; (void)count;
    AbortNotRewritten("verified stream delegate has an x86-only pointer ABI");
#endif
}
void StreamText(uintptr_t* scope, const void* data, int count)
{
#if defined(_M_IX86)
    reinterpret_cast<LoadSaveWriter7D8520*>(scope)->Write(static_cast<int32_t>(reinterpret_cast<uintptr_t>(data)), count);
#else
    (void)scope; (void)data; (void)count;
    AbortNotRewritten("verified stream delegate has an x86-only pointer ABI");
#endif
}
void CopyUnlessSame(WorldString& to, WorldString* from)
{ if (&to != from) AssignString(to, from->begin, from->end); }
}
int WorldSaveManager::SaveChallenge(const uint16_t* data, int count, const uint16_t* identifier) // 0x00AB05F0
{
    WorldPath path;
    path.Clear();
    auto res = CallGameFunction<void*, 0x0072F1C0>();
    auto base = CallGameVirtual<const uint16_t*, 9>(res);
    if (!base || !*base) return 4;
    path.Assign(base);
    if (!CallGameFunction<uint8_t, 0x004DCFF0>(Text(path))) return 4;
    auto manager = CallGameFunction<WorldManagerFields*, 0x00984550>();
    const auto category = GameData<char, 0x010015FC>();
    WorldString passport{nullptr, nullptr, nullptr, category};
    CallGameMethod<void, 0x0040B700>(&passport, GameData<uint16_t, 0x010647F4>());
    WorldString file{nullptr, nullptr, nullptr, category};
    // The manager's field at +0x100 is a pointer, not a dword integer.
    auto passportRoot = manager->passportRoot;
    CallGameMethod<void, 0x0040B700>(&file, passportRoot);
    auto id = CallGameFunction<int64_t, 0x007CFE40>(identifier);
    char decimal[64];
    std::snprintf(decimal, sizeof decimal, "%lld", static_cast<long long>(id));
    auto idString = EmptyString();
    WorldString temp;
    CopyUnlessSame(idString, CallGameFunction<WorldString*, 0x004F3750>(&temp, decimal, -1, 0));
    FreeString(temp);
    WorldString joined;
    auto result = Join(&joined, &file, &idString);
    result = Append(&temp, result, GameData<uint16_t, 0x00FB20FC>());
    CopyUnlessSame(file, result);
    FreeString(temp); FreeString(joined);
    CallGameFunction<void, 0x004DCFF0>(file.begin);
    CopyUnlessSame(file, Join(&joined, &file, &passport));
    FreeString(joined);
    auto backup = EmptyString();
    result = Join(&temp, &file, &backup);
    result = Append(&joined, result, GameData<uint16_t, 0x010647E4>());
    CopyUnlessSame(backup, result);
    FreeString(joined); FreeString(temp);
    bool hadOriginal = false;
    if (CallGameFunction<uint8_t, 0x004DBBA0>(file.begin)) {
        hadOriginal = true;
        CopyFileDbc30(reinterpret_cast<const wchar_t*>(file.begin), reinterpret_cast<const wchar_t*>(backup.begin), 1);
    }
    void* db = nullptr;
    auto block = Allocate(920, GameData<char, 0x01064788>());
    if (block) {
        db = CallGameMethod<void*, 0x004A8F70>(block, file.begin, 0);
        if (db) CallGameVirtual<void, 0>(static_cast<uint8_t*>(db) + sizeof(void*));
    }
    int error = 0;
    if (db && CallGameVirtual<uint8_t, 1>(db)) {
        CallGameMethod<void, 0x005169D0>(db, 1);
        if (CallGameVirtual<uint8_t, 6>(db, 2, 2, 0)) {
            uintptr_t scope[6];
            CallGameMethod<void, 0x007D76F0>(scope, db, 1);
            if (!CallGameMethod<uint8_t, 0x007D8290>(scope, GameData<uint8_t, 0x01182E40>(), 1)) {
                CallGameMethod<void, 0x007D7760>(scope);
                Release(db);
                CallGameMethod<void, 0x004EF330>(&backup);
                CallGameMethod<void, 0x004EF330>(&idString);
                CallGameMethod<void, 0x004EF330>(&file);
                CallGameMethod<void, 0x004EF330>(&passport);
                return 9; // EA bypasses rollback and close on this early failure.
            }
            StreamInt(scope, 1);
            const char signature[3] = {'S', '3', 0};
            StreamBytes(scope, signature, 3);
            CallGameMethod<void, 0x00565730>(scope, 4);
            StreamInt(scope, 4);
            StreamRaw(scope, data1Begin, ByteSpan(data1Begin, data1End));
            StreamRaw(scope, data2Begin, ByteSpan(data2Begin, data2End));
            auto hash = Hash4FA500(reinterpret_cast<const uint8_t*>(data), static_cast<int32_t>(static_cast<uint32_t>(count) * 2), ~0u, ~0u, 1);
            StreamHash(scope, hash);
            StreamText(scope, data, count);
            CallGameMethod<void, 0x007D7840>(scope);
            if (!(scope[5] & 0xFF)) {
                CallGameMethod<void, 0x007D7760>(scope);
                Release(db);
                CallGameMethod<void, 0x004EF330>(&backup);
                CallGameMethod<void, 0x004EF330>(&idString);
                CallGameMethod<void, 0x004EF330>(&file);
                CallGameMethod<void, 0x004EF330>(&passport);
                return 10;
            }
            CallGameVirtual<void, 7>(db);
            CallGameMethod<void, 0x007D7760>(scope);
        } else error = 15;
        CallGameVirtual<void, 2>(db);
        Release(db);
        db = nullptr;
    } else error = 17;
    if (error && hadOriginal) {
        CallGameFunction<void, 0x004DBBF0>(backup.begin, file.begin, 1);
        CallGameFunction<void, 0x00AAF4B0>(file.begin);
    }
    if (CallGameFunction<uint8_t, 0x004DBBA0>(backup.begin))
        CallGameFunction<void, 0x004DBBD0>(backup.begin);
    if (db) Release(db);
    FreeString(backup); FreeString(idString); FreeString(file); FreeString(passport);
    return error;
}
int LoadChallengeData(int data, int count, int identifier) // 0x00742BB0
{
    auto resources = CallGameFunction<void*, 0x007692B0>();
    if (!resources) return 0;
    auto manager = CallGameFunction<WorldSaveManager*, 0x00AAB380>();
    if (!manager) return 0;
    auto text = CallGameVirtual<const uint16_t*, 22>(resources, data);
    if (!text || !*text) return 0;
    auto id = CallGameVirtual<const uint16_t*, 22>(resources, identifier);
    if (!*text) return 0; // EA rechecks the data, not the resolved identifier.
    return manager->SaveChallenge(text, count, id);
}
int LoadChallengeDataThunk(int data, int count, int identifier) // 0x00742CC0
{
    return LoadChallengeData(data, count, identifier);
}
uint8_t WorldSaveManager::Load(const void* input) // 0x00AB14B0
{
    const auto isSpecial = special;
    if ((!input && !isSpecial) || busy) return 0;
    auto host = *GameData<WorldHost*, 0x011ECBC4>();
    if (!host) return 0;
    host->loadStarted = 1;
    host->loading = 1;
    WorldPath path;
    path.Clear();
    struct Input { uintptr_t pad[2]; const uint16_t* name; };
    WorldLoadData paths;
    paths.Init(input ? static_cast<const Input*>(input)->name : nullptr);
    if (CallGameMethod<int, 0x00AAE950>(this, &paths, 1, isSpecial)) {
        CallGameMethod<void, 0x00AABB10>(&paths);
        return 0; // EA leaves host->loading set on this and the next early exit.
    }
    uint8_t fallback = 0;
    WorldPath* selected;
    if (isSpecial || CallGameFunction<uint8_t, 0x004DBBA0>(Text(paths.path2))) selected = &paths.path2;
    else {
        fallback = CallGameFunction<uint8_t, 0x004DBBA0>(Text(paths.path3));
        if (!fallback) { CallGameMethod<void, 0x00AABB10>(&paths); return 0; }
        selected = &paths.path3;
    }
    path = *selected;
    uint8_t success = 0;
    void* db = nullptr;
    auto block = Allocate(920, GameData<char, 0x01064788>());
    auto value = block ? CallGameMethod<void*, 0x004A8F70>(block, Text(path), 0) : nullptr;
    CallGameMethod<void, 0x004A0A00>(&db, value);
    if (CallGameVirtual<uint8_t, 1>(db)) {
        if (CallGameVirtual<uint8_t, 6>(db, 1, 3, 0)) {
            if (input && !isSpecial) CallGameMethod<void, 0x00CF3C70>(&request, input);
            success = CallGameMethod<uint8_t, 0x00AACF60>(this, db);
            CallGameVirtual<void, 7>(db);
        } else if (CallGameFunction<uint8_t, 0x004DBBA0>(Text(path))) {
            CallGameFunction<void, 0x004DBCA0>(Text(path));
        }
        CallGameVirtual<void, 2>(db);
    }
    if (db) Release(db);
    if (!success && request) {
        struct Request { void* vtable; uint32_t references; };
        auto old = static_cast<Request*>(request);
        request = nullptr;
        if (--old->references == 0) {
            old->references = 1;
            CallGameVirtual<void, 0>(old, 1);
        }
    }
    if (!isSpecial && !fallback) {
        auto manager = CallGameFunction<WorldManagerFields*, 0x00984550>();
        if (manager) {
            auto secondary = manager->secondary;
            if (secondary) CallGameMethod<void, 0x009CBF20>(secondary, 0, nullptr, 1);
        }
        if (!CallGameMethod<uint8_t, 0x00AB0E80>(this, paths.worldName.begin, 1)) success = 0;
        CallGameFunction<void, 0x00AAEAC0>(1, 1);
    }
    host->loading = 0;
    CallGameMethod<void, 0x00AABB10>(&paths);
    return success;
}
}
