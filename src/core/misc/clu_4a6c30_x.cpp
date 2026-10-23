#include "ts3/misc/clu_4a6c30_x.h"
#include <utility>
#include <limits>
#include <cmath>
#include <cwctype>
#include <cwchar>
namespace ts3::misc {
namespace {
bool Less(const ResourceSortKey& a, const ResourceSortKey& b)
{
    if (a.type != b.type) return a.type < b.type;
    if (a.group != b.group) return a.group < b.group;
    if (a.high != b.high) return a.high < b.high;
    return a.low < b.low;
}
bool Equal(const ResourceSortKey& a, const ResourceSortKey& b)
{ return a.type == b.type && a.group == b.group && a.low == b.low && a.high == b.high; }
}
// 0x004AF0E0: separate stores preserve overlapping-source behavior.
uint32_t ResourceKeyTarget::AssignKey(const void* input)
{
    const auto source = static_cast<const ResourceSortKey*>(input);
    key.type = source->type;
    key.group = source->group;
    key.low = source->low;
    key.high = source->high;
    return key.high;
}
// 0x004AF530
ResourceSortKey* KeyInsertionSort(ResourceSortKey* first, ResourceSortKey* last)
{
    ResourceSortKey* result = last;
    if (first == last) return result;
    for (auto i = first + 1; i != last; ++i) {
        auto value = *i;
        result = i;
        while (result != first && Less(value, result[-1])) {
            *result = result[-1]; --result;
        }
        *result = value;
    }
    return result;
}
// 0x004AF5E0
void KeyUnguardedInsertion(ResourceSortKey* first, ResourceSortKey* last)
{
    for (auto i = first; i != last; ++i) {
        auto value = *i;
        auto hole = i;
        while (Less(value, hole[-1])) { *hole = hole[-1]; --hole; }
        *hole = value;
    }
}
// 0x004AF670
ResourceSortKey* KeyAdjacentFind(ResourceSortKey* first, ResourceSortKey* last)
{
    if (first == last) return last;
    for (auto next = first + 1; next != last; first = next++)
        if (Equal(*first, *next)) return first;
    return last;
}
// 0x004AF6B0
ResourceSortKey* KeyMedian(ResourceSortKey* a, ResourceSortKey* b, ResourceSortKey* c)
{
    if (Less(*a, *b)) {
        if (Less(*b, *c)) return b;
        if (Less(*a, *c)) return c;
        return a;
    }
    if (Less(*a, *c)) return a;
    if (Less(*b, *c)) return c;
    return b;
}
// 0x004AF7A0
uint32_t KeyPushHeap(ResourceSortKey* base, int32_t top, int32_t hole, const ResourceSortKey* value)
{
    while (hole > top) {
        const int32_t parent = (hole - 1) / 2;
        if (!Less(base[parent], *value)) break;
        base[hole] = base[parent]; hole = parent;
    }
    base[hole].type = value->type;
    base[hole].group = value->group;
    const uint32_t result = value->group;
    base[hole].low = value->low;
    base[hole].high = value->high;
    return result;
}
// 0x004AF840
ResourceSortKey* KeyUnique(ResourceSortKey* first, ResourceSortKey* last)
{
    auto out = KeyAdjacentFind(first, last);
    if (out == last) return last;
    for (auto i = out + 1; i != last; ++i)
        if (!Equal(*out, *i)) *++out = *i;
    return out + 1;
}
// 0x004AF8B0
uint32_t KeyAdjustHeap(ResourceSortKey* base, int32_t top, int32_t count, int32_t hole, const ResourceSortKey* value)
{
    int32_t child = 2 * hole + 2;
    while (child < count) {
        if (Less(base[child], base[child - 1])) --child;
        base[hole] = base[child]; hole = child; child = 2 * child + 2;
    }
    if (child == count) { base[hole] = base[child - 1]; hole = child - 1; }
    return KeyPushHeap(base, top, hole, value);
}
// 0x004AF970
ResourceSortKey* KeyPartition(ResourceSortKey* first, ResourceSortKey* last, const ResourceSortKey* pivot)
{
    const auto value = *pivot;
    for (;;) {
        while (Less(*first, value)) ++first;
        do { --last; } while (Less(value, *last));
        if (first >= last) return first;
        std::swap(*first, *last); ++first;
    }
}
// 0x004AFA40
void KeyMakeHeap(ResourceSortKey* first, ResourceSortKey* last)
{
    const auto count = static_cast<int32_t>(last - first);
    if (count < 2) return;
    auto parent = (count - 2) / 2 + 1;
    do { --parent; auto value = first[parent]; KeyAdjustHeap(first, parent, count, parent, &value); } while (parent);
}
// 0x004AFAB0
int32_t KeySortHeap(ResourceSortKey* first, ResourceSortKey* last)
{
    auto count = static_cast<int32_t>(last - first);
    while (count > 1) {
        auto value = *--last;
        *last = *first;
        KeyAdjustHeap(first, 0, --count, 0, &value);
    }
    return count;
}
// 0x004AFB30
int32_t KeyPartialSort(ResourceSortKey* first, ResourceSortKey* middle, ResourceSortKey* last)
{
    KeyMakeHeap(first, middle);
    for (auto i = middle; i < last; ++i) {
        if (Less(*i, *first)) {
            auto value = *i; *i = *first;
            KeyAdjustHeap(first, 0, static_cast<int32_t>(middle - first), 0, &value);
        }
    }
    return KeySortHeap(first, middle);
}
// 0x004AFBE0: keep the original residue return, including a negative-depth no-op.
int32_t KeyIntroLoop(ResourceSortKey* first, ResourceSortKey* last, int32_t depth)
{
    int32_t result = static_cast<int32_t>((last - first) * 16);
    while ((last - first) > 28 && depth > 0) {
        auto pivot = KeyMedian(first, first + (last - first) / 2, last - 1);
        auto split = KeyPartition(first, last, pivot);
        --depth;
        result = KeyIntroLoop(split, last, depth);
        last = split;
    }
    if (depth == 0) return KeyPartialSort(first, last, last);
    return result;
}
// 0x004AFCD0: 0x004AFC60 belongs to another unit, as does the empty-tail move.
int32_t KeySortUnique(ResourceKeyVector* vector)
{
    CallGameFunction<void, 0x004AFC60>(vector->first, vector->last);
    auto end = KeyUnique(vector->first, vector->last);
    CallGameFunction<void, 0x00C60180>(vector->last, vector->last, end);
    vector->last = end;
    return static_cast<int32_t>(end - vector->first);
}
// 0x004B3740
uint32_t InterpolatePackedColor(uint32_t a, uint32_t b, float fraction)
{
    uint32_t result = 0;
    for (unsigned shift = 0; shift != 32; shift += 8) {
        const auto from = static_cast<int32_t>((a >> shift) & 255);
        const auto to = static_cast<int32_t>((b >> shift) & 255);
        const float delta = static_cast<float>(to - from) * fraction;
        const int32_t step = delta >= -2147483648.0f && delta < 2147483648.0f
            ? static_cast<int32_t>(delta) : std::numeric_limits<int32_t>::min();
        result |= ((static_cast<uint32_t>(from) + static_cast<uint32_t>(step)) & 255) << shift;
    }
    return result;
}
}

namespace ts3::misc {
// 0x004AEC10
const char16_t* FindHexSuffix(const char16_t* first, const char16_t* last, uint8_t underscore, uint8_t allow64)
{
    if (underscore) for (auto p = first; p != last; ++p) if (*p == u'_') first = p + 1;
    if (last - first < 8) return nullptr;
    if (first[0] == u'0' && (first[1] == u'x' || first[1] == u'X')) first += 2;
    else if (!std::iswdigit(*first)) return nullptr;
    auto p = first;
    while (p != last && std::iswxdigit(*p)) ++p;
    const auto count = p - first;
    return p == last && (count == 8 || (count == 16 && allow64)) ? first : nullptr;
}
// 0x004AEB70: 0x007CFCC0 is outside this unit.
uint32_t FoldedWideHash(const char16_t* text, uint32_t length)
{
    const auto value = CallGameFunction<uint32_t, 0x007CFCC0>(text, length);
    return (value & 0x00ffffffu) ^ (value >> 24);
}
// 0x004AECC0
void* QueryResourceInterface(void* object)
{
    if (!object) return nullptr;
    auto subobject = static_cast<void**>(object) + 1;
    return CallGameVirtual<void*, 3>(subobject, uint32_t{0x0226a1de});
}
// 0x004AF200: output type/group are written even if later syntax is rejected.
uint8_t ParseResourceKey(const char16_t* text, ResourceSortKey* output)
{
#if defined(_M_IX86)
    auto p = reinterpret_cast<const wchar_t*>(text);
    wchar_t* end = nullptr;
    output->low = static_cast<uint32_t>(std::wcstoul(p, &end, 16));
    if (*end != L':') return 0;
    output->high = static_cast<uint32_t>(std::wcstoul(end + 1, &end, 16));
    if (*end != L':') return 0;
    uint64_t instance = CallGameFunction<uint64_t, 0x004FED70>(end + 1, &end, 16);
    if (*end == L'\'' && !(instance >> 32)) {
        auto quote = end;
        const auto low = static_cast<uint32_t>(std::wcstoul(end + 1, &end, 16));
        instance = (instance << 32) | low;
        if (end - quote != 9) return 0;
    }
    if (*end && *end != L' ') return 0;
    output->type = static_cast<uint32_t>(instance);
    output->group = static_cast<uint32_t>(instance >> 32);
    return 1;
#else
    (void)text; (void)output;
    AbortNotRewritten("wide CRT resource parser"); return 0;
#endif
}
// 0x004B40B0
int32_t ResourceVisitorTarget::VisitSlot9(void* visitor)
{ return CallGameVirtual<int32_t, 9>(visitor, this); }
// 0x004B4110
int32_t ResourceVisitorTarget::VisitSlot10(void* visitor)
{ return CallGameVirtual<int32_t, 10>(visitor, this); }
// 0x004B41E0: both predicates use AL only.
int32_t ResourceVisitContainer::VisitChildren(void* visitor)
{
    if (CallGameVirtual<uint8_t, 3>(visitor, this))
        for (auto p = sentinel.next; p != &sentinel; p = p->next)
            if (!CallGameVirtual<uint8_t, 6>(p->item, visitor)) break;
    return CallGameVirtual<int32_t, 4>(visitor, this);
}
// 0x004E0BA0
ResourceTreeNode* RotateResourceRight(ResourceTreeNode* node, ResourceTreeNode* root)
{
    auto child = node->right;
    node->right = child->left;
    if (child->left) child->left->parent = node;
    child->parent = node->parent;
    if (node == root) root = child;
    else if (node == node->parent->left) node->parent->left = child;
    else node->parent->right = child;
    child->left = node; node->parent = child;
    return root;
}
// 0x004E0BF0
ResourceTreeNode* RotateResourceLeft(ResourceTreeNode* node, ResourceTreeNode* root)
{
    auto child = node->left;
    node->left = child->right;
    if (child->right) child->right->parent = node;
    child->parent = node->parent;
    if (node == root) root = child;
    else if (node == node->parent->right) node->parent->right = child;
    else node->parent->left = child;
    child->right = node; node->parent = child;
    return root;
}
}

#include "ts3/hash.h"
#include <algorithm>
namespace ts3::misc {
// 0x004A6C30
uint32_t PackedFileQueries::QueryIndex(uint32_t argument)
{
    recordMutex.Lock();
    const uint32_t result = index ? CallGameVirtual<uint32_t, 8>(index, argument) : 0;
    recordMutex.Unlock();
    return result;
}
// 0x004A6EE0
uint32_t PackedFileQueries::LargestFreeBlock()
{
    recordMutex.Lock();
    const auto result = reinterpret_cast<FreeSpaceQueries*>(&freeSpace)->LargestBlock();
    recordMutex.Unlock();
    return result;
}
// 0x004AA510
uint32_t FreeSpaceQueries::LargestBlock()
{
    uint32_t result = 0;
    for (const auto& pair : blocks) {
        const auto start = static_cast<uint32_t>(pair.first);
        auto count = static_cast<uint32_t>(pair.second);
        // Deliberately unsigned wrapping end calculation, then signed tail comparison.
        if (start + count > limit) {
            if (start > limit) continue;
            count = limit - start;
        }
        result = std::max(result, count);
    }
    if (total < static_cast<int32_t>(limit)) result = std::max(result, limit - static_cast<uint32_t>(total));
    return result;
}
// 0x004A76A0: return register is unspecified on the null path; callers use this as void.
void ResourceNodeMap::Destroy40(ResourceTreeNode* node)
{
    while (node) {
        Destroy40(node->right);
        auto next = node->left;
        CallGameVirtual<void, 3>(allocator, node, sizeof(ResourceMapNode));
        node = next;
    }
}
// 0x004B0B00: node payload has sixteen bytes, with pointer-bearing links growing on x64.
void ResourceNodeMap::Destroy32(ResourceTreeNode* node)
{
    while (node) {
        Destroy32(node->right);
        auto next = node->left;
        CallGameVirtual<void, 3>(allocator, node, sizeof(ResourceTreeNode) + size_t{16});
        node = next;
    }
}
// 0x004A8400
ResourceTreeNode** ResourceNodeMap::Erase(ResourceTreeNode** output, ResourceTreeNode* node)
{
    --size;
    auto next = CallGameFunction<ResourceTreeNode*, 0x004E0B20>(node);
    EraseResourceTree(node, &anchor);
    CallGameVirtual<void, 3>(allocator, node, sizeof(ResourceMapNode));
    *output = next;
    return output;
}
// 0x004A8C40
ResourceTreeNode** ResourceNodeMap::InsertAt(ResourceTreeNode** output, ResourceTreeNode* parent, const ResourceMapValue* value, uint8_t forceLeft)
{
    const uint32_t side = forceLeft || parent == &anchor || Less(value->key, reinterpret_cast<ResourceMapNode*>(parent)->value.key) ? 0 : 1;
    auto node = CallGameVirtual<ResourceMapNode*, 2>(allocator, sizeof(ResourceMapNode), flags, name);
    // The original checks payload address != null (not node != null). Keep that oddity.
    if (reinterpret_cast<uintptr_t>(node) != uintptr_t{0} - offsetof(ResourceMapNode, value)) {
        node->value.key = value->key;
        node->value.record = value->record;
        node->value.padding = value->padding;
    }
    CallGameFunction<void, 0x004E0C40>(&node->links, parent, &anchor, side);
    ++size;
    *output = &node->links;
    return output;
}
// 0x004A8F10
ResourceTreeNode** ResourceNodeMap::Insert(ResourceTreeNode** output, const ResourceMapValue* value, uint32_t unused)
{
    (void)unused;
    auto parent = &anchor;
    for (auto node = anchor.parent; node;) {
        parent = node;
        node = Less(value->key, reinterpret_cast<ResourceMapNode*>(node)->value.key) ? node->left : node->right;
    }
    InsertAt(output, parent, value, 0);
    return output;
}
namespace {
// The game's 64-bit conversion is not rewritten yet. char16_t keeps the source ABI
// independent of the host wchar_t width; only the x86 game bridge interprets it.
uint64_t ReadHex64(const char16_t* text, const char16_t** end)
{ return CallGameFunction<uint64_t, 0x004FED70>(text, end, int32_t{16}); }
uint32_t ReadHex32(const char16_t* text, const char16_t** end)
{
    // Windows CRT wide unsigned long is 32 bits on x86 and x64.
    static_assert(sizeof(wchar_t) == sizeof(char16_t), "Windows UTF-16 CRT required");
    wchar_t* stop = nullptr;
    auto result = static_cast<uint32_t>(std::wcstoul(reinterpret_cast<const wchar_t*>(text), &stop, 16));
    if (end) *end = reinterpret_cast<const char16_t*>(stop);
    return result;
}
const char16_t* WideEnd(const char16_t* text)
{ while (*text) ++text; return text; }
}
// 0x004AECE0
uint8_t ParseResourceName(ResourceSortKey* output, const char16_t* text, uint32_t type, uint32_t group, void* manager, void* context)
{
    if (!text || !*text) return 0;
    output->low = type;
    auto separator = text;
    auto name = text;
    auto end = text;
    while (*end && *end != u'.') ++end;
    const auto extension = *end ? end + 1 : nullptr;
    output->high = group;
    for (auto p = text; p != end; ++p) if (*p == u'!') {
        separator = p; name = p + 1; output->high = UINT32_MAX; break;
    }
    uint64_t instance = UINT64_MAX;
    if (name != end) {
        const auto hex = FindHexSuffix(name, end, 1, 1);
        instance = hex ? ReadHex64(hex, nullptr) : hash::Fnv64LowerW(name, static_cast<uint32_t>(end - name));
    }
    output->type = static_cast<uint32_t>(instance);
    output->group = static_cast<uint32_t>(instance >> 32);
    if (separator != text) {
        auto hex = FindHexSuffix(text, separator, 1, 0);
        output->high = hex ? ReadHex32(hex, nullptr) : FoldedWideHash(text, static_cast<uint32_t>(separator - text));
    }
    auto notify = manager;
    if (!output->low) {
        output->low = UINT32_MAX;
        if (extension && *extension) {
            auto extEnd = WideEnd(extension);
            if (auto hex = FindHexSuffix(extension, extEnd, 0, 0)) output->low = ReadHex32(hex, nullptr);
            else {
                int32_t resolved = -1;
                if (context) if (auto resolver = QueryResourceInterface(context))
                    resolved = CallGameVirtual<int32_t, 24>(resolver, extension);
                if (resolved == -1) {
                    if (!notify) notify = CallGameFunction<void*, 0x004AFD20>();
                    if (notify) resolved = CallGameVirtual<int32_t, 36>(notify, extension);
                    if (resolved == -1) output->low = CallGameFunction<uint32_t, 0x007CFCC0>(extension, static_cast<uint32_t>(extEnd - extension));
                    else output->low = static_cast<uint32_t>(resolved);
                } else output->low = static_cast<uint32_t>(resolved);
            }
        }
    }
    if (notify) CallGameVirtual<void, 34>(notify, output, text);
    return 1;
}
// 0x004AF2E0: preserve apostrophe OR (not concatenation), empty endpoint maxima,
// and the unusual explicit-zero upper instance => UINT64_MAX rule.
uint8_t ParseResourceRange(const char16_t* const* text, ResourceKeyRange* output)
{
    auto p = *text;
    uint32_t lo[2]{}, hi[2]{};
    for (unsigned part = 0; part != 2; ++part) {
        auto begin = p;
        if (*p != u'-') lo[part] = ReadHex32(p, &p);
        if (*p == u':') hi[part] = p == begin ? UINT32_MAX : lo[part];
        else {
            if (*p != u'-') return 0;
            begin = ++p;
            hi[part] = ReadHex32(p, &p);
            if (p == begin) hi[part] = UINT32_MAX;
            if (*p != u':') return 0;
        }
        ++p;
    }
    auto begin = p;
    uint64_t min = 0;
    if (*p != u'-') min = ReadHex64(p, &p);
    if (*p == u'\'') {
        begin = ++p;
        const auto value = ReadHex64(p, &p);
        min = (static_cast<uint64_t>(static_cast<uint32_t>(min) | static_cast<uint32_t>(value >> 32)) << 32) | static_cast<uint32_t>(value);
        if (p - begin != 8) return 0;
    }
    uint64_t max;
    if (!*p || *p == u' ') max = p == begin ? UINT64_MAX : min;
    else {
        if (*p != u'-') return 0;
        max = ReadHex64(p + 1, &p);
        if (*p == u'\'') {
            begin = ++p;
            const auto value = ReadHex64(p, &p);
            max = (static_cast<uint64_t>(static_cast<uint32_t>(max) | static_cast<uint32_t>(value >> 32)) << 32) | static_cast<uint32_t>(value);
            if (p - begin != 8) return 0;
        }
        if (!max) max = UINT64_MAX;
        if (*p && *p != u' ') return 0;
    }
    if (lo[0] > hi[0] || lo[1] > hi[1] || min > max) return 0;
    output->minType = lo[0]; output->maxType = hi[0];
    output->minGroup = lo[1]; output->maxGroup = hi[1];
    output->minInstance = min; output->maxInstance = max;
    return 1;
}
}

namespace ts3::misc {
// 0x004E0E20. right/left are EA's physical +0/+4 links; anchor.parent is root.
// Keep the otherwise incidental EAX pointer, not merely the resulting root.
ResourceTreeNode* EraseResourceTree(ResourceTreeNode* node, ResourceTreeNode* anchor)
{
    ResourceTreeNode* result = node;
    ResourceTreeNode* child;
    ResourceTreeNode* parent;
    if (!node->left) child = node->right;
    else if (!node->right) child = node->left;
    else {
        result = node->right;
        while (result->left) result = result->left;
        child = result->right;
    }
    if (result == node) {
        parent = result->parent;
        if (child) child->parent = parent;
        if (node == anchor->parent) anchor->parent = child;
        else {
            result = node->parent;
            if (node == result->left) result->left = child;
            else result->right = child;
        }
        if (node == anchor->left) {
            if (node->right) {
                auto minimum = child;
                result = child->left;
                while (result) { minimum = result; result = result->left; }
                anchor->left = minimum;
            } else { result = node->parent; anchor->left = result; }
        }
        if (node == anchor->right) {
            if (node->left) {
                result = child->right;
                auto maximum = child;
                while (result) { maximum = result; result = result->right; }
                anchor->right = maximum;
            } else anchor->right = node->parent;
        }
    } else {
        node->left->parent = result;
        result->left = node->left;
        if (result == node->right) parent = result;
        else {
            parent = result->parent;
            if (child) child->parent = parent;
            parent->left = child;
            result->right = node->right;
            node->right->parent = result;
        }
        if (node == anchor->parent) anchor->parent = result;
        else if (node == node->parent->left) node->parent->left = result;
        else node->parent->right = result;
        result->parent = node->parent;
        std::swap(result->black, node->black);
    }
    if (node->black == 1) {
        while (child != anchor->parent) {
            if (child && child->black != 1) break;
            result = parent->left;
            if (child == result) {
                result = parent->right;
                if (!result->black) {
                    result->black = 1; parent->black = 0;
                    anchor->parent = RotateResourceRight(parent, anchor->parent);
                    result = parent->right;
                }
                if ((result->left && result->left->black != 1) || (result->right && result->right->black != 1)) {
                    if (!result->right || result->right->black == 1) {
                        result->left->black = 1; result->black = 0;
                        anchor->parent = RotateResourceLeft(result, anchor->parent);
                        result = parent->right;
                    }
                    result->black = parent->black; parent->black = 1;
                    if (result->right) result->right->black = 1;
                    result = RotateResourceRight(parent, anchor->parent);
                    anchor->parent = result;
                    break;
                }
            } else {
                if (!result->black) {
                    result->black = 1; parent->black = 0;
                    anchor->parent = RotateResourceLeft(parent, anchor->parent);
                    result = parent->left;
                }
                if ((result->right && result->right->black != 1) || (result->left && result->left->black != 1)) {
                    if (!result->left || result->left->black == 1) {
                        result->right->black = 1; result->black = 0;
                        anchor->parent = RotateResourceRight(result, anchor->parent);
                        result = parent->left;
                    }
                    result->black = parent->black; parent->black = 1;
                    if (result->left) result->left->black = 1;
                    result = RotateResourceLeft(parent, anchor->parent);
                    anchor->parent = result;
                    break;
                }
            }
            child = parent;
            result->black = 0;
            const bool atRoot = parent == anchor->parent;
            parent = parent->parent;
            if (atRoot) break;
        }
        if (child) child->black = 1;
    }
    return result;
}
}
