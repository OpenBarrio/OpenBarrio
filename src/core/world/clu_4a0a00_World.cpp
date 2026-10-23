#include "ts3/world/clu_4a0a00_World.h"
#include <cstring>
#include "ts3/world/clu_4ec290_World.h"
#include "ts3/cas/clu_4a6710_CAS.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"
#if defined(_M_IX86)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace ts3::world {
WorldRef* WorldRef::Assign(WorldRefObject* value)
{
    // 004A0A00: callbacks observe the old pointer during retain and the new one during release.
    auto* old = object;
    if (value != old) {
        if (value) CallGameVirtual<void, 0>(&value->reference);
        object = value;
        if (old) CallGameVirtual<void, 1>(&old->reference);
    }
    return this;
}
uint8_t WorldDbOptions::SetOptions(int32_t value, float seconds)
{
    // 0x004A69B0
    kind = value;
    if (seconds >= 0.0f) delay = seconds;
    return 1;
}
uintptr_t WorldResourceService()
{
    // 0072F1C0
#if defined(_M_IX86)
    return *reinterpret_cast<uintptr_t*>(0x011D3E28);
#else
    AbortNotRewritten("WorldResourceService global");
    return 0;
#endif
}
uint8_t ResolveWorldKey(const WorldResourceKey* key, void* output)
{
    // 0093B9A0
    WorldResourceKey resolved = *key;
    if (resolved.words[2] != 0x02DC343Fu) {
        if (resolved.words[2] != 0x319E4F1Du) return 0;
#if defined(_M_IX86)
        auto* manager = *reinterpret_cast<void**>(0x011ED034);
#else
        void* manager = nullptr;
        AbortNotRewritten("world key manager global");
#endif
        if (!manager) return 0;
        const auto* record = CallGameMethod<const uint8_t*, 0x00C8EC50>(manager, key);
        if (!record) return 0;
        std::memcpy(&resolved, record + 136, sizeof resolved);
    }
    return CallGameVirtual<uint8_t, 0>(reinterpret_cast<void*>(WorldResourceService()), &resolved, 0x02DC3450u, output);
}
uint8_t WorldKeyPair::Resolve(void* firstOutput, void* secondOutput)
{
    // 009460B0: second is skipped on failed first resolution unless its instance is zero.
    auto* resource = reinterpret_cast<void*>(reinterpret_cast<void*>(WorldResourceService()));
    if (!first.words[2] && !second.words[2]) return 0;
    if (!first.words[2] ||
        CallGameVirtual<uint8_t, 0>(resource, &first, 0xC807312Au, firstOutput) ||
        !(first.words[0] | first.words[1])) {
        if (second.words[2])
            CallGameVirtual<void, 0>(resource, &second, 0xC807312Au, secondOutput);
    }
    return 1;
}
uint32_t WorldResetService::Reset()
{
    // 0x009E52A0
    CallGameMethod<void, 0x009E5310>(this);
    const uint32_t result = CallGameFunction<uint32_t, 0x009E6350>(reinterpret_cast<void*>(WorldResourceService()));
    return (result & 0xFFFFFF00u) | 1u;
}
int32_t WorldTravelService::Open(void* path, const char16_t* filename, uint8_t flag)
{
    // 009CBF20: the helper takes this AND the explicit record pointer (Hex-Rays omits this).
    CallGameMethod<int32_t, 0x009CBD80>(this, transientRecord, static_cast<void*>(nullptr), static_cast<const char16_t*>(nullptr), 0);
#if defined(_M_IX86)
    EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(criticalSection));
#else
    AbortNotRewritten("WorldTravelService critical section");
#endif
    CallGameMethod<int32_t, 0x009CBD80>(this, travelRecord, path, u"TravelDB.package", 1);
#if defined(_M_IX86)
    LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(criticalSection));
#endif
    const auto result = CallGameMethod<int32_t, 0x009CBD80>(this, worldRecord, path, filename, 0);
    enabled = flag;
    return result;
}
namespace {
struct WorldUserKey {
    char* begin;
    char* end;
    char* capacity;
    const char* owner;
};
TS3_X86_SIZE(WorldUserKey, 16);
char* EmptyWorldString()
{
#if defined(_M_IX86)
    return reinterpret_cast<char*>(0x011C7E54);
#else
    AbortNotRewritten("world empty string global");
    return nullptr;
#endif
}
void FreeUserKey(const WorldUserKey& key)
{
    // Signed byte-capacity comparison, unlike the wide-string helpers' masked delta.
    if (static_cast<intptr_t>(reinterpret_cast<uintptr_t>(key.capacity) - reinterpret_cast<uintptr_t>(key.begin)) > 1 && key.begin)
        scratch::Free4E3F70(key.begin);
}
}
uint8_t OpenWorldDatabase(const char16_t* source, WorldRef* output, int32_t mode, int32_t flags)
{
    // 00C63500
    auto* manager = CallGameFunction<void*, 0x004AFD20>();
    if (!manager) return 0;
    WorldRefObject* db;
    if (!source) {
        auto* memory = CallGameFunction<void*, 0x004E41F0>(176, "World/WorldDB", 0, 0, 0, 0);
        db = memory ? CallGameMethod<WorldRefObject*, 0x0072CC60>(memory, 0) : nullptr;
        output->Assign(db);
    } else {
        if (!CallGameFunction<uint8_t, 0x00996590>(source)) {
            auto* memory = CallGameFunction<void*, 0x004E41F0>(176, "World/WorldDB", 0, 0, 0, 0);
            db = memory ? CallGameMethod<WorldRefObject*, 0x0072CC60>(memory, 0) : nullptr;
            output->Assign(db);
            CallGameVirtual<void, 11>(output->object, source);
        } else {
            auto* empty = EmptyWorldString();
            WorldUserKey key{empty, empty, empty + 1, "World/userKey"};
            if (!CallGameFunction<uint8_t, 0x00995690>(&key)) {
                FreeUserKey(key);
                return 0;
            }
            auto* memory = CallGameFunction<void*, 0x004E41F0>(936, "World/WorldDB", 0, 0, 0, 0);
            db = memory ? CallGameMethod<WorldRefObject*, 0x00996680>(memory, key.begin, source, 0) : nullptr;
            output->Assign(db);
            FreeUserKey(key);
        }
        if (!CallGameVirtual<uint8_t, 1>(output->object) ||
            !CallGameVirtual<uint8_t, 6>(output->object, mode, flags, 0)) return 0;
    }
    db = output->object;
    if (!db) return 0;
    return CallGameVirtual<uint8_t, 13>(manager, 1, db, 0);
}
namespace {
void CloseDesignerDatabase(WorldRef& db)
{
    CallGameVirtual<void, 2>(db.object);
    // Reload after callback: it may have cleared the owner's reference.
    db.Assign(nullptr);
}
void InitDesignerDatabase(WorldDesigner* self, int32_t mode, void* manager)
{
    auto* root = CallGameVirtual<const char16_t*, 9>(reinterpret_cast<void*>(WorldResourceService()));
    if (!root) return;
    char16_t path[260];
    cas::CopyUtf16(reinterpret_cast<uint16_t*>(path), reinterpret_cast<const uint16_t*>(root), 260);
    CallGameFunction<void, 0x004FF470>(path, u"userPresets.package", 260);
    if (!CallGameFunction<uint8_t, 0x004DBBA0>(path) && mode == 1) return;
    auto* memory = CallGameFunction<void*, 0x004E41F0>(920, "World/ObjectDesignerService/Init", 0, 0, 0, 0);
    auto* obj = memory ? CallGameMethod<WorldRefObject*, 0x004A8F70>(memory, path, 0) : nullptr;
    self->database.Assign(obj);
    if (!self->database.object || !CallGameVirtual<uint8_t, 1>(self->database.object)) return;
    reinterpret_cast<WorldDbOptions*>(self->database.object)->SetOptions(23, 0.25f);
    if (!CallGameVirtual<uint8_t, 6>(self->database.object, mode, 6, 0)) {
        if (mode != 3 || !CallGameVirtual<uint8_t, 6>(self->database.object, 1, 6, 0))
            CloseDesignerDatabase(self->database);
    }
    if (self->database.object && !CallGameVirtual<uint8_t, 13>(manager, 1, self->database.object, 0))
        CloseDesignerDatabase(self->database);
}
}
void WorldDesigner::EnsureDatabase(int32_t mode)
{
    // 00B31E00 is void: disabled path leaves incoming EAX untouched, and its callers
    // inspect database, not EAX. No fabricated return value for that live-in register.
    if (!enabled) return;
    auto* manager = CallGameFunction<void*, 0x004AFD20>();
    if (database.object) {
        auto oldMode = CallGameVirtual<int32_t, 8>(database.object);
        if (oldMode != mode && oldMode != 3) {
            CallGameVirtual<void, 7>(database.object);
            if (!CallGameVirtual<uint8_t, 6>(database.object, mode, 6, 0) && mode == 3)
                CallGameVirtual<void, 6>(database.object, 1, 6, 0);
        }
        return;
    }
    InitDesignerDatabase(this, mode, manager);
}
void WorldDesigner::AddColor(const WorldResourceKey* key)
{
    // 00B324D0
    CallGameFunction<void*, 0x004AFD20>();
    EnsureDatabase(3);
    if (database.object) CallGameVirtual<void, 16>(database.object, key);
}
uint8_t WorldDesigner::RemoveColor(const WorldResourceKey* key)
{
    // 00B32500 contains an inline copy of EnsureDatabase(1); retain its extra manager call.
    CallGameFunction<void*, 0x004AFD20>();
    EnsureDatabase(1);
    if (!database.object) return 0;
    CallGameVirtual<void, 24>(database.object);
    return CallGameVirtual<uint8_t, 13>(database.object, key, static_cast<void*>(nullptr), 1, 6, 1, 0);
}
void WorldDesigner::RemoveDesign(const WorldResourceKey* key, uint32_t low, uint32_t high)
{
    // 00B326D0
    CallGameFunction<void*, 0x004AFD20>();
    void* record = nullptr;
    EnsureDatabase(3);
    if (database.object && CallGameVirtual<uint8_t, 13>(database.object, key, &record, 3, 2, 1, 0)) {
        auto* data = CallGameVirtual<void*, 6>(record);
        CallGameVirtual<void, 14>(data, low, high);
        CallGameVirtual<void, 15>(database.object, record);
        CallGameVirtual<void, 2>(record);
    }
}
void ScriptAddColor(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    // 00787E90
    auto* designer = CallGameFunction<WorldDesigner*, 0x00B319C0>();
    WorldResourceKey key{{c, d, a, b}};
    designer->AddColor(&key);
}
uint8_t ScriptRemoveColor(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    // 00787ED0
    auto* designer = CallGameFunction<WorldDesigner*, 0x00B319C0>();
    WorldResourceKey key{{c, d, a, b}};
    return designer->RemoveColor(&key);
}
void ScriptRemoveDesign(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t id)
{
    // 00787E20
    auto* designer = CallGameFunction<WorldDesigner*, 0x00B319C0>();
    auto* host = CallGameFunction<void*, 0x007692B0>();
    uint32_t high = 0, low = 0;
    CallGameVirtual<void, 28>(host, id, &low, &high);
    WorldResourceKey key{{c, d, a, b}};
    designer->RemoveDesign(&key, low, high);
}
namespace {
struct WorldText {
    char16_t* begin;
    char16_t* end;
    char16_t* capacity;
    const char* owner;
};
TS3_X86_SIZE(WorldText, 16);
struct MapReference {
    void* table;
    int32_t references;
};
TS3_X86_OFFSET(MapReference, references, 4);
void ReleaseMissingMap(MapReference* ref)
{
#if defined(_M_IX86)
    if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(&ref->references)) == 0) {
        InterlockedExchange(reinterpret_cast<volatile LONG*>(&ref->references), 1);
        CallGameVirtual<void, 0>(ref, 1);
    }
#else
    (void)ref; AbortNotRewritten("map reference atomics");
#endif
}
int32_t WideCapacityBytes(const WorldText& s)
{
    return static_cast<int32_t>((reinterpret_cast<uintptr_t>(s.capacity) - reinterpret_cast<uintptr_t>(s.begin)) & ~uintptr_t(1));
}
void FreeWorldText(const WorldText& s)
{
    if (WideCapacityBytes(s) > 2 && s.begin) scratch::Free4E3F70(s.begin);
}
}
char* WorldDesigner::MapTagText(uint32_t low, uint32_t high)
{
    // 00B33300
    auto* registry = CallGameFunction<void*, 0x00938B50>();
    auto* ref = CallGameMethod<MapReference*, 0x00939560>(registry, low, high);
    if (!ref) return nullptr;
#if defined(_M_IX86)
    InterlockedIncrement(reinterpret_cast<volatile LONG*>(&ref->references));
    auto* mapType = *reinterpret_cast<void**>(0x011E8FC4);
#else
    void* mapType = nullptr;
    AbortNotRewritten("map reference global");
#endif
    auto* map = CallGameMethod<void*, 0x00935770>(ref, mapType);
    if (!map) { ReleaseMissingMap(ref); return nullptr; }
    auto release = [&] { CallGameMethod<void, 0x007460F0>(&ref); };
    CallGameMethod<void, 0x0094A280>(map, &cachedBegin);
    if (cachedBegin != cachedEnd) { auto* result = cachedBegin; release(); return result; }
    auto* keySource = CallGameMethod<const WorldResourceKey*, 0x00949D30>(map);
    auto key = *keySource;
    key.words[2] = 0x0333406Cu;
    void* record = nullptr;
    if (!CallGameVirtual<uint8_t, 1>(reinterpret_cast<void*>(WorldResourceService()), &key, &record, 0, 1, 6)) {
        release(); return nullptr;
    }
    auto* stream = CallGameVirtual<void*, 6>(record);
    const uint32_t size = CallGameVirtual<uint32_t, 7>(stream);
    auto close = [&] { CallGameVirtual<void, 8>(record); CallGameVirtual<void, 2>(record); };
    if (!size) { close(); release(); return nullptr; }
#if defined(_M_IX86)
    auto* allocator = *reinterpret_cast<void**>(0x011CB864);
#else
    void* allocator = nullptr;
    AbortNotRewritten("map text allocator global");
#endif
    auto* text = CallGameMethod<uint8_t*, 0x004E69D0>(allocator, size + 1, 0);
    CallGameVirtual<void, 12>(stream, text, size);
    text[size] = 0;
    close();
    if (size > 3 && text[0] == 0xEF && text[1] == 0xBB && text[2] == 0xBF) {
        std::memmove(text, text + 3, size - 3);
        // EA writes three zeros, not just the string terminator, after stripping BOM.
        std::memset(text + size - 3, 0, 3);
    }
    CallGameMethod<void, 0x0094A960>(map, text);
    CallGameMethod<void, 0x004E5920>(allocator, text);
    CallGameMethod<void, 0x0094A280>(map, &cachedBegin);
    auto* result = cachedBegin;
    release();
    return result;
}
int32_t ScriptForceMapTagTick(uint32_t low, uint32_t high)
{
    // 007950C0
    auto* designer = CallGameFunction<WorldDesigner*, 0x00B319C0>();
    auto* host = CallGameFunction<void*, 0x007692B0>();
    auto* text = designer->MapTagText(low, high);
    if (!text) return 0;
    WorldText argument{nullptr, nullptr, nullptr, "World/WorldHost"};
    CallGameMethod<void, 0x0040B700>(&argument, text);
    struct Message {
        void* table;
        uintptr_t value;
        uint16_t flags, type;
        uint8_t data[12];
    } message;
    TS3_X86_OFFSET(Message, flags, 8);
    TS3_X86_SIZE(Message, 24);
#if defined(_M_IX86)
    message.table = reinterpret_cast<void*>(0x01006FB4);
#else
    AbortNotRewritten("map-tag message vtable");
#endif
    message.flags = 11; message.type = 19;
    CallGameMethod<void, 0x004F5800>(&message, 19, 9, &argument, static_cast<int32_t>(sizeof argument), 1);
    int32_t result;
    CallGameVirtual<void, 20>(host, &message, &result);
#if defined(_M_IX86)
    message.table = reinterpret_cast<void*>(0x00FB96B4);
#endif
    if (message.flags & 4) CallGameMethod<void, 0x004F55A0>(&message, 0);
    FreeWorldText(argument);
    return result;
}
namespace {
template <class T> T* WorldGlobal(unsigned address)
{
#if defined(_M_IX86)
    return *reinterpret_cast<T**>(static_cast<uintptr_t>(address));
#else
    (void)address; AbortNotRewritten("world global"); return nullptr;
#endif
}
WorldWideText EmptyWideText()
{
    auto* empty = reinterpret_cast<char16_t*>(EmptyWorldString());
    return {empty, empty, empty + 1, "World/LoadSaveManager"};
}
int32_t CapacityBytes(const WorldWideText& text)
{
    return static_cast<int32_t>((reinterpret_cast<uintptr_t>(text.capacity)-reinterpret_cast<uintptr_t>(text.begin)) & ~uintptr_t(1));
}
uintptr_t FreeWideTextValue(const WorldWideText& text)
{
    if (CapacityBytes(text) > 2 && text.begin) return reinterpret_cast<uintptr_t>(scratch::Free4E3F70(text.begin));
    return reinterpret_cast<uintptr_t>(text.begin);
}
void AssignWide(WorldWideText* target, const WorldWideText* source)
{
    if (source != target) CallGameMethod<void, 0x00408E20>(target, source->begin, source->end);
}
void AppendPath(SaveWorldPath* path, const char16_t* text)
{ CallGameMethod<void, 0x004EDD60>(path, text, 0); }
void SetBase(SaveWorldPath* path, const char16_t* text)
{ reinterpret_cast<WorldPath*>(path)->Splice23(reinterpret_cast<const uint16_t*>(text), 0); }
void SetExtension(SaveWorldPath* path, const char16_t* text)
{ CallGameMethod<void, 0x004EDD20>(path, text, 0); }
const char16_t* PathText(const SaveWorldPath* path)
{ return CallGameMethod<const char16_t*, 0x00F626F0>(path); }
struct TravelOwner { uint8_t unknown[1636]; WorldTravelService* travel; };
TS3_X86_OFFSET(TravelOwner, travel, 1636);
TS3_X86_SIZE(TravelOwner, 1640);
}
uint8_t GetCurrentSaveWorldPath(SaveWorldPath* output)
{
    // 00AAB3F0
    const auto* path = CallGameVirtual<const char16_t*, 9>(reinterpret_cast<void*>(WorldResourceService()));
    if (!path || !*path) return 0;
    reinterpret_cast<WorldPath*>(output)->Assign(reinterpret_cast<const uint16_t*>(path));
    return CallGameFunction<uint8_t, 0x004DCFF0>(PathText(output));
}
int32_t WorldSaveContext::Validate(WorldSavePaths* paths, void* unused)
{
    // 00AACD80 ignores both the ECX receiver and its second stack argument.
    (void)unused;
    auto text = EmptyWideText();
    AssignWide(&text, &paths->directory);
    const auto bytes = static_cast<int32_t>(reinterpret_cast<uintptr_t>(text.end) - reinterpret_cast<uintptr_t>(text.begin));
    const auto count = bytes >> 1;
    int32_t result;
    if (!count) result = 5;
    else if (text.begin == text.end || static_cast<uint32_t>(count) > 242) result = 6;
    else result = CallGameFunction<uint8_t, 0x004EE030>(text.begin, 5) ? 0 : 7;
    FreeWideTextValue(text);
    return result;
}
uintptr_t WorldSavePaths::Build(const SaveWorldPath* sourcePath, const WorldWideText* worldCode)
{
    // 00AAC6B0
    auto text = EmptyWideText();
    const auto* name = PathText(sourcePath);
    const auto* end = name; while (*end) ++end;
    CallGameMethod<void, 0x00408E20>(&text, name, end);
    source = *sourcePath;
    AppendPath(&source, directory.begin);
    const auto* dirEnd = directory.begin; while (*dirEnd) ++dirEnd;
    CallGameMethod<void, 0x00408B90>(&text, directory.begin, dirEnd);
    const char16_t separator[] = u"\\";
    CallGameMethod<void, 0x00408B90>(&text, separator, separator + 1);
    reinterpret_cast<WorldPath*>(&root)->Assign(reinterpret_cast<const uint16_t*>(text.begin));
    current = root; fallback = root; meta = root;
    AssignWide(&code, worldCode);
    SetBase(&current, worldCode->begin); SetExtension(&current, u"nhd");
    SetBase(&meta, u"Meta"); SetExtension(&meta, u"data");
    if (worldName.begin != worldName.end) {
        auto converted = EmptyWideText();
        WorldWideText temporary;
        auto* result = CallGameFunction<WorldWideText*, 0x004F3890>(&temporary, &worldName);
        AssignWide(&converted, result); FreeWideTextValue(temporary);
        SetBase(&fallback, converted.begin); SetExtension(&fallback, u"nhd");
        FreeWideTextValue(converted);
    } else if (worldCode->begin == worldCode->end) {
        // A Win32 file wrapper at +0, 400-byte ANSI header buffer at +552,
        // and an inline 1040-byte path at +752. None contains pointers accessed here.
        alignas(void*) uint8_t file[552];
        char header[400];
        CallGameMethod<void, 0x004DB910>(file, static_cast<void*>(nullptr));
        SaveWorldPath input = root;
        AppendPath(&input, WorldGlobal<char16_t>(0x01182DE4));
        CallGameMethod<void, 0x004DB3E0>(file, PathText(&input));
        CallGameMethod<void, 0x004DB4C0>(file, 1, 6, 1, 0);
        CallGameFunction<void, 0x004DD970>(file, header, 200);
        WorldWideText temporary;
        auto* result = CallGameFunction<WorldWideText*, 0x004F3750>(&temporary, header, -1, 0);
        AssignWide(&code, result); FreeWideTextValue(temporary);
        CallGameMethod<void, 0x004DB600>(file);
        SetBase(&fallback, code.begin); SetExtension(&fallback, u"nhd");
        CallGameMethod<void, 0x004DB9D0>(file);
    }
    chosen = CallGameFunction<uint8_t, 0x004DBBA0>(PathText(&source)) ? source : fallback;
    return FreeWideTextValue(text);
}
int32_t WorldSaveContext::Prepare(WorldSavePaths* paths, uint8_t clearCode, uint8_t useCurrentWorld)
{
    // 00AAE950: ECX is a real context receiver, not a stdcall free function.
    SaveWorldPath root;
    reinterpret_cast<WorldPath*>(&root)->Clear();
    if (useCurrentWorld) {
        if (!GetCurrentSaveWorldPath(&root)) return 4;
        CallGameMethod<void, 0x00AAC680>(paths, WorldGlobal<void>(0x01182DDC));
    } else if (!CallGameMethod<uint8_t, 0x00AAB430>(this, &root)) return 4;
    auto code = EmptyWideText();
    auto* directory = WorldGlobal<void>(0x011ECBC4);
    if (directory) {
        const auto status = Validate(paths, directory);
        if (status) return status; // empty temporary is not freed on this path in EA.
        WorldWideText temporary;
        auto* result = CallGameFunction<WorldWideText*, 0x00AAE860>(&temporary);
        AssignWide(&code, result); FreeWideTextValue(temporary);
    }
    if (clearCode && code.begin != code.end) { *code.begin = 0; code.end = code.begin; }
    paths->Build(&root, &code);
    FreeWideTextValue(code);
    return 0;
}
uintptr_t ConfigureTravelDatabase(uint8_t enabled, uint8_t flag)
{
    // 00AAEAC0
    auto* owner = CallGameFunction<TravelOwner*, 0x00984550>();
    if (!owner || !owner->travel) return reinterpret_cast<uintptr_t>(owner);
    auto* travel = owner->travel;
    if (!enabled) return static_cast<uint32_t>(travel->Open(nullptr, nullptr, 1));
    SaveWorldPath path;
    reinterpret_cast<WorldPath*>(&path)->Clear();
    GetCurrentSaveWorldPath(&path); // EA ignores this failure and continues with initialized empty path.
    CallGameMethod<void, 0x004EDB00>(&path, WorldGlobal<void>(0x01182DDC));
    AppendPath(&path, u"TravelDB.package");
    auto code = EmptyWideText();
    WorldWideText temporary;
    auto* result = CallGameFunction<WorldWideText*, 0x00AAE860>(&temporary);
    AssignWide(&code, result); FreeWideTextValue(temporary);
    const auto index = CallGameMethod<int32_t, 0x0083B9D0>(&code, u".", -1);
    if (index != -1) CallGameMethod<void, 0x00408B30>(&code, index, -1);
    CallGameMethod<void, 0x004F1F50>(&code, u"ExportDB.package");
    travel->Open(&path, code.begin, flag);
    return FreeWideTextValue(code);
}
uint8_t ResetTravelDatabase()
{
    // 00AAEFA0
    auto* owner = CallGameFunction<TravelOwner*, 0x00984550>();
    if (owner && owner->travel) owner->travel->Open(nullptr, nullptr, 1);
    SaveWorldPath path;
    reinterpret_cast<WorldPath*>(&path)->Clear();
    if (!GetCurrentSaveWorldPath(&path)) return 0;
    AppendPath(&path, WorldGlobal<char16_t>(0x01182DDC));
    if (CallGameFunction<uint8_t, 0x004DBF60>(PathText(&path))) {
        unsigned attempts = 0;
        while (!CallGameFunction<uint8_t, 0x004DCC00>(PathText(&path), 1)) {
            int32_t delay = 500; CallGameFunction<void, 0x004E1320>(&delay);
            if (++attempts >= 10) return 0;
        }
    }
    unsigned attempts = 0;
    while (!CallGameFunction<uint8_t, 0x004DCD40>(PathText(&path))) {
        int32_t delay = 500; CallGameFunction<void, 0x004E1320>(&delay);
        if (++attempts >= 10) return 0;
    }
    ConfigureTravelDatabase(1, 1);
    return 1;
}
} // namespace ts3::world
