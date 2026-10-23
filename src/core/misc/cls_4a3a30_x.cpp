#include "ts3/misc/cls_4a3a30_x.h"
#include "ts3/misc/cls_408760_x.h"
#include "ts3/misc/cls_4a6630_x.h"
#include "ts3/resourcesystem/clu_4a3640_ResourceSystem.h"
#include "ts3/scratch/clu_4a49d0_Scratch.h"
#include <cstring>
#include <cwchar>
#include <cwctype>

namespace ts3::misc {
namespace {
using services::ServiceNodeBase;
using services::ServiceNode;
using services::ServiceKey;
struct NameValue { services::ServiceString text; uint32_t type; };
struct NameNode { ServiceNodeBase base; NameValue value; };
TS3_X86_OFFSET(NameNode, value.type, 32);
struct Guard {
    thread::Mutex& m;
    explicit Guard(thread::Mutex& value) : m(value) { m.Lock(thread::kTimeoutNone); }
    ~Guard() { m.Unlock(); }
};
bool Equal(const wchar_t* a, const wchar_t* b) {
    while (*a && std::towlower(*a) == std::towlower(*b)) { ++a; ++b; }
    return std::towlower(*a) == std::towlower(*b);
}
ServiceNodeBase* Next(ServiceNodeBase* node) { return CallGameFunction<ServiceNodeBase*, 0x004E0B20>(node); }
ServiceNode* Find(services::ServiceTree& tree, const ServiceKey* key) {
    ServiceNodeBase* out;
    CallGameMethod<ServiceNodeBase**, 0x0057A7D0>(&tree, &out, key);
    return reinterpret_cast<ServiceNode*>(out);
}
void Join(wchar_t* out, const wchar_t* base, const wchar_t* path) {
    resourcesystem::JoinPath(reinterpret_cast<char16_t*>(out), reinterpret_cast<const char16_t*>(base), reinterpret_cast<const char16_t*>(path));
}
// 0x4EDA20 constructs a fixed 520-character path parser; these are UTF-16
// characters, not native pointer slots. The parser's methods remain game code.
struct ParsedPath {
    wchar_t data[520];
    explicit ParsedPath(const wchar_t* source) { CallGameMethod<void, 0x004EDA20>(data, source); }
    wchar_t* Name() { return CallGameMethod<wchar_t*, 0x004ED5E0>(data); }
    void Stem(wchar_t* out) { CallGameMethod<void, 0x004ED670>(data, out, 256); }
};
}
uintptr_t ResourceManConfig::SetEntryCallback(uintptr_t callback) // 004A3A30
{
    unknown7C = callback;
    return callback;
}
uint64_t ResourceManConfig::FileTime(const ServiceKey* key) // 004A3BD0
{
    wchar_t relative[260], full[260];
    if (!CallGameVirtual<uint8_t, 26>(this, key, relative)) return 0;
    Join(full, path.begin, relative);
    if (!CallGameFunction<uint8_t, 0x004DBBA0>(full)) return 0;
    return CallGameFunction<uint64_t, 0x004DBCF0>(full, 2);
}
uint8_t ResourceManConfig::ResolveFile(const wchar_t* source, void* output) // 004A3C50
{
    Guard lock(mutex);
    ParsedPath parsed(source);
    wchar_t stem[260];
    wchar_t* name = parsed.Name();
    parsed.Stem(stem);
    if (unknown7C) {
        using Callback = uint8_t (*)(const wchar_t*, const wchar_t*, const wchar_t*, void*);
        return reinterpret_cast<Callback>(unknown7C)(stem, name, source, output);
    }
    return CallGameFunction<uint8_t, 0x004AECE0>(output, source, 0, generation, 0, this);
}
uint32_t ResourceManConfig::TypeForName(const wchar_t* name) // 004A3EA0
{
    Guard lock(mutex);
    if (*name == L'.') ++name;
    for (auto* n = entryTree.anchor.left; n != &entryTree.anchor; n = Next(n)) {
        const auto& v = reinterpret_cast<NameNode*>(n)->value;
        if (Equal(v.text.begin, name)) return v.type;
    }
    wchar_t* end;
    // wcstoul is 32-bit on both target Windows ABIs. Empty text succeeds as zero.
    uint32_t result = static_cast<uint32_t>(std::wcstoul(name, &end, 16));
    return *end ? 0xFFFFFFFFu : result;
}
const wchar_t* ResourceManConfig::NameForType(uint32_t type) // 004A3F50
{
    Guard lock(mutex);
    for (auto* n = entryTree.anchor.left; n != &entryTree.anchor; n = Next(n)) {
        const auto& v = reinterpret_cast<NameNode*>(n)->value;
        if (v.type == type) return v.text.begin;
    }
    return nullptr;
}
uint8_t ResourceManConfig::BindProvider(uint32_t add, void* value, uint32_t notify) // 004A3FB0
{
    // EA tests only the low byte of the two boolean arguments.
    if (!static_cast<uint8_t>(add)) {
        if (value && value != provider) return 0;
        if (static_cast<uint8_t>(notify)) { provider = nullptr; return 1; }
        return CallGameVirtual<uint8_t, 13>(provider, 0, this, 0);
    }
    if (provider && value != provider) CallGameVirtual<uint8_t, 18>(this, 0, provider, notify);
    if (!value) return 0;
    if (!static_cast<uint8_t>(notify)) return CallGameVirtual<uint8_t, 13>(value, 1, this, 0);
    provider = value;
    for (auto* n = entryTree.anchor.left; n != &entryTree.anchor; n = Next(n)) {
        const auto& v = reinterpret_cast<NameNode*>(n)->value;
        CallGameVirtual<void, 38>(provider, v.type, v.text.begin);
    }
    return 1;
}
uint8_t ResourceManConfig::CopyRecordPath(const ServiceKey* key, wchar_t* out) // 004A46D0
{
    Guard lock(mutex);
    auto* n = Find(recordTree, key);
    if (&n->base == &recordTree.anchor) return 0;
    const wchar_t* in = n->value.payload.path.begin;
    do { *out++ = *in; } while (*in++);
    return 1;
}
uint8_t ResourceManConfig::RemoveRecord(void* record) // 004A48D0
{
    Guard lock(mutex);
    auto* key = CallGameVirtual<const ServiceKey*, 4>(record);
    auto* n = Find(recordTree, key);
    if (&n->base == &recordTree.anchor) return 0;
    return RemoveDdfRecord(reinterpret_cast<DdfList*>(&n->value.payload), static_cast<DdfRecord*>(record));
}
uint8_t ResourceManConfig::RegisterType(const wchar_t* name, uint32_t type) // 004A4DD0
{
    Guard lock(mutex);
    if (*name == L'.') ++name;
    if (!*name) return 0;
    // Match the game's two temporary owning strings, including capacity-based frees.
    services::ServiceString first{nullptr, nullptr, nullptr, const_cast<char*>("ResourceSystem/ResourceMan/DDF")};
    CallGameMethod<void, 0x0040B700>(&first, name);
    NameValue copy{{nullptr, nullptr, nullptr, first.allocator}, type};
    const auto length = first.end - first.begin;
    CallGameMethod<void, 0x00408830>(&copy.text, static_cast<uint32_t>(length + 1));
    std::memmove(copy.text.begin, first.begin, length * sizeof(wchar_t));
    copy.text.end = copy.text.begin + length;
    *copy.text.end = 0;
    if (first.capacity - first.begin > 1 && first.begin) CallGameFunction<void, 0x004E3F70>(first.begin);
    ServiceNodeBase* out;
    CallGameMethod<void, 0x004F03D0>(&entryTree, &out, &copy, uint32_t(0));
    if (provider) CallGameVirtual<void, 38>(provider, type, name);
    if (copy.text.capacity - copy.text.begin > 1 && copy.text.begin) CallGameFunction<void, 0x004E3F70>(copy.text.begin);
    return 1;
}
uint8_t ResourceManConfig::DeleteRecord(const ServiceKey* key) // 004A52A0
{
    Guard lock(mutex);
    if (!active) return 0;
    auto* n = Find(recordTree, key);
    if (&n->base == &recordTree.anchor || n->value.payload.links.next != &n->value.payload.links) return 0;
    wchar_t full[260];
    Join(full, path.begin, n->value.payload.path.begin);
    if (!CallGameFunction<uint8_t, 0x004DBBD0>(full)) return 0;
    ServiceNodeBase* out;
    recordTree.Erase(&out, n);
    return 1;
}
int32_t ResourceManConfig::Scan(void* keys) // 004A5340
{
    if (!active || !CallGameFunction<uint8_t, 0x004DBF60>(path.begin)) return 0;
    reinterpret_cast<services::ResourceMan*>(this)->LoadDefinitions();
    struct Link { Link* next; Link* previous; uint32_t unknown; wchar_t* name; };
    struct List { Link* next; Link* previous; void* allocator; uint32_t count; const char* name; } list;
    list.next = list.previous = reinterpret_cast<Link*>(&list);
    list.allocator = CallGameFunction<void*, 0x004D9AD0>();
    list.count = 0;
    list.name = "ResourceSystem/ResourceMan/EntryList";
    uintptr_t scanner[4]{};
    CallGameMethod<void, 0x004DA3C0>(scanner, path.begin, &list, static_cast<void*>(nullptr), 2, 0x100000);
    uint32_t total = 0;
    for (auto* p = list.next; p != reinterpret_cast<Link*>(&list); p = p->next)
        total += CallGameVirtual<int32_t, 22>(this, p->name, keys);
    CallGameMethod<void, 0x00456CC0>(&list);
    return static_cast<int32_t>(total);
}
int32_t ResourceManConfig::UpdateFile(const wchar_t* source, void* keys) // 004A6100
{
    Guard lock(mutex);
    ParsedPath parsed(source);
    wchar_t stem[260], full[260];
    wchar_t* name = parsed.Name();
    ServiceKey key{};
    parsed.Stem(stem);
    if (Equal(stem, L"DDFMap") || Equal(stem, L"DDFList") || CallGameVirtual<int32_t, 24>(this, name) == -1) return 0;
    if (!static_cast<uint8_t>(reinterpret_cast<DdfManager*>(this)->Resolve(reinterpret_cast<uintptr_t>(stem), reinterpret_cast<uintptr_t>(name), reinterpret_cast<const char16_t*>(source), reinterpret_cast<uintptr_t>(&key)))) return 0;
    auto* n = Find(recordTree, &key);
    Join(full, path.begin, source);
    uint8_t exists = CallGameFunction<uint8_t, 0x004DBBA0>(full);
    if (&n->base == &recordTree.anchor) {
        if (!exists) return 0;
        reinterpret_cast<services::ResourceMan*>(this)->AddRecord(&key, full, source);
    } else if (exists) {
        const uint64_t stamp = CallGameFunction<uint64_t, 0x004DBCF0>(full, 2);
        if (n->value.payload.low == static_cast<uint32_t>(stamp) && n->value.payload.high == static_cast<uint32_t>(stamp >> 32)) return 0;
        n->value.payload.low = static_cast<uint32_t>(stamp);
        n->value.payload.high = static_cast<uint32_t>(stamp >> 32);
    } else if (n->value.payload.links.next == &n->value.payload.links) {
        ServiceNodeBase* out;
        recordTree.Erase(&out, n);
    }
    if (keys) static_cast<scratch::KeyList*>(keys)->Append(reinterpret_cast<const scratch::ResourceKey*>(&key));
    return 1;
}
void* DeletePackedSecondary(void* secondary, uint32_t flags) // 004A84F0
{
    auto* primary = reinterpret_cast<PackedFileCore*>(static_cast<char*>(secondary) - sizeof(void*));
    return primary->Delete(flags);
}
}
