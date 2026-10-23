#include "ts3/world/clu_4a47a0_World.h"
#include <initializer_list>
#include <cstring>
#include <cmath>
#include <limits>
#if defined(_M_IX86)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
namespace ts3::world {
// 0x006A0850. ECX is carried through recursion but never dereferenced.
void FreeTree(TreeNode* node) {
    while (node) {
        FreeTree(node->child);
        TreeNode* next = node->sibling;
        CallGameFunction<void, 0x004e3f70>(node);
        node = next;
    }
}
// 0x00B40CD0
int32_t WorldTree::Clear() {
    FreeTree(root);
    last = &first;
    first = &first;
    root = nullptr;
    flag = 0;
    count = 0;
    return 0;
}
// 0x007D8E50
void DetachDatabase(void* database, void* manager, uint8_t notify) {
    if (!database) return;
    if (!manager) manager = CallGameFunction<void*, 0x004afd20>();
    if (manager) CallGameVirtual<void, 13>(manager, 0, database, 0);
    if (notify) {
        void* keys = CallGameFunction<void*, 0x0072f1c0>();
        if (keys) {
            KeyList list{nullptr, nullptr, nullptr, "World/KeyList"};
            CallGameVirtual<void, 12>(database, &list, 0);
            CallGameVirtual<void, 21>(keys, &list);
            if (list.begin) CallGameFunction<void, 0x004e3f70>(list.begin);
        }
    }
    CallGameVirtual<void, 7>(database);
    CallGameVirtual<void, 2>(database);
}
// 0x004A47A0
int32_t WorldCleanup::Cleanup() {
    CallGameMethod<void, 0x004a4460>(this);
    FreeTree(root);
    return CallGameMethod<int32_t, 0x004dabd0>(embedded);
}
// 0x009A2FE0
void DatabasePair::Cleanup() {
    for (void** slot : {&second, &first}) {
        if (*slot) {
            void* manager = CallGameFunction<void*, 0x004afd20>();
            DetachDatabase(*slot, manager, 0);
            // The detach callback can replace/clear this field: reread it.
            void* db = *slot;
            if (db) {
                *slot = nullptr;
                CallGameVirtual<void, 1>(static_cast<void**>(db) + 1);
            }
        }
    }
}
namespace {
void* Fill(WorldTree& tree, ShortRange* range) {
    tree.Clear();
    void* result = range;
    for (auto p = range->begin; p != range->end; ++p) {
        int32_t key = *p; // sign extension is observable for negative short IDs.
        void* iterator[2];
        result = CallGameMethod<void*, 0x006a1be0>(&tree, iterator, &key, 0);
    }
    return result;
}
template<unsigned Address> void* Propagate(WorldSelections* self, ShortRange* range, void* result) {
    auto cursor = self->cursor;
    auto page = self->page;
    auto end = self->pageEnd;
    const auto finish = self->finish;
    while (cursor != finish) {
        result = CallGameMethod<void*, Address>(*cursor, range);
        if (++cursor == end) {
            cursor = *++page;
            end = cursor + 64; // original pages hold 64 pointers, not 256 x64 bytes.
        }
    }
    return result;
}
}
// 0x00ADC810
void* WorldSelections::SetSelections(ShortRange* range) {
    return Propagate<0x006a5d10>(this, range, Fill(selections, range));
}
// 0x00ADC8C0
void* WorldSelections::SetExclusions(ShortRange* range) {
    return Propagate<0x006a5e00>(this, range, Fill(exclusions, range));
}
// 0x00ADC970
void* WorldSelections::SetFinalSelections(ShortRange* range) {
    return Fill(finalSelections, range);
}
// 0x006C8080
uint8_t Candidate::Collect(WorldTree* output) {
    if (!CallGameVirtual<uint8_t, 6>(validator)) return 0;
    if (related) {
        int32_t key = related->key;
        void* iterator[2];
        CallGameMethod<void*, 0x006a1be0>(output, iterator, &key, 0);
    }
    return 1;
}
// 0x006C6D40
uint8_t CandidateTable::Collect(int32_t key, WorldTree* output) {
    output->Clear();
    auto bucket = buckets;
    while (!*bucket) ++bucket; // the sentinel bucket makes this scan terminate.
    auto node = *bucket;
    const auto sentinel = buckets[count];
    uint8_t found = 0;
    while (node != sentinel) {
        for (auto p = node->range->begin; p != node->range->end; ++p) {
            Candidate* candidate = *p;
            if (candidate->key == key && candidate->kind == 2 && candidate->Collect(output)) found = 1;
        }
        node = node->next;
        while (!node) node = *++bucket;
    }
    return found;
}
// 0x00C65A20
void WorldLotOwner::RemoveLot(WorldLot* lot, int32_t destroy) {
    if (!lot) return;
    if (active->idLow == lot->idLow && active->idHigh == lot->idHigh) return;
    if (lot->parent) {
        CallGameVirtual<void, 21>(lot->parent, lot, destroy);
        return;
    }
    auto p = lots.begin;
    while (p != lots.end && *p != lot) ++p;
    if (p == lots.end) return;
    WorldLot* removed = *p;
    if (removed) CallGameVirtual<void, 0>(removed);
    CallGameMethod<void, 0x0080cb00>(&lots, p);
    if (static_cast<uint8_t>(destroy)) {
        CallGameVirtual<void, 23>(removed);
        CallGameMethod<void, 0x00c65830>(this, removed);
    }
    if (removed) CallGameVirtual<void, 1>(removed);
}
// 0x00C604B0
uint8_t WorldMemoryDatabase::Create() {
    void* manager = CallGameFunction<void*, 0x004afd20>();
    if (!manager) return 0;
    if (database) DetachDatabase(database, manager, 1);
    void* storage = CallGameFunction<void*, 0x004e41f0>(160, "World/MemoryDB", 0, 0, 0, 0);
    void* replacement = storage ? CallGameMethod<void*, 0x0072b8f0>(storage, 0, "World/MemoryDB") : nullptr;
    void* old = database;
    if (replacement != old) {
        if (replacement) CallGameVirtual<void, 0>(static_cast<void**>(replacement) + 1);
        database = replacement;
        if (old) CallGameVirtual<void, 1>(static_cast<void**>(old) + 1);
    }
    if (database) {
        if (CallGameVirtual<uint8_t, 1>(database) &&
            CallGameVirtual<uint8_t, 6>(database, 3, 6, 0) &&
            CallGameVirtual<uint8_t, 13>(manager, 1, database, 0)) return 1;
        DetachDatabase(database, manager, 1);
        void* db = database;
        if (db) {
            database = nullptr;
            CallGameVirtual<void, 1>(static_cast<void**>(db) + 1);
        }
    }
    return 0;
}
namespace {
TerrainEdgeSet EdgeSet() {
    TerrainEdgeSet set{};
    set.name = "World/LotTerrainLODCreator/edgePoints";
    // The self links must be initialized after the returned object reaches its final address.
    return set;
}
uintptr_t DestroyEdges(TerrainEdgeSet& set, uintptr_t result) {
    auto node = set.tree.root;
    while (node) {
        FreeTree(node->child);
        auto next = node->sibling;
        result = CallGameFunction<uintptr_t, 0x004e3f70>(node);
        node = next;
    }
    return result;
}
void AppendPoint(TerrainPointVector& vector, const TerrainPoint& point, bool x87Copy) {
    auto destination = vector.end;
    if (reinterpret_cast<uintptr_t>(destination) >= reinterpret_cast<uintptr_t>(vector.capacity)) {
        CallGameMethod<void, 0x00405e40>(&vector, destination, &point);
    } else {
        vector.end = reinterpret_cast<TerrainPoint*>(reinterpret_cast<uintptr_t>(destination) + sizeof(TerrainPoint));
        if (destination) {
            if (x87Copy) {
                // The original copies the center via x87: signaling NaNs become quiet.
                uint32_t bits[3]; std::memcpy(bits, &point, sizeof bits);
                for (auto& b : bits) if ((b & 0x7f800000u) == 0x7f800000u && (b & 0x7fffffu)) b |= 0x400000u;
                std::memcpy(destination, bits, sizeof bits);
            } else std::memcpy(destination, &point, sizeof point);
        }
    }
}
}
// 0x00AF5BD0. The nominal EAX return is scratch state; callers use only the set.
uintptr_t TerrainQuad::CollectCorners(TerrainEdgeSet* points, const TerrainRect* rect) {
    uintptr_t result = bounds.maxX;
    if (bounds.maxX < rect->minX || bounds.minX > rect->maxX || bounds.maxY < rect->minY) return result;
    result = bounds.minY;
    if (bounds.minY > rect->maxY) return result;
    if (leaf) {
        for (unsigned i = 0; i != 4; ++i) {
            const uint32_t x = i < 2 ? bounds.minX : bounds.maxX;
            const uint32_t y = i & 1 ? bounds.maxY : bounds.minY;
            result = y;
            if (x >= rect->minX && y >= rect->minY && x <= rect->maxX && y <= rect->maxY) {
                uint32_t key = x + (y << 7);
                void* iterator[2];
                result = CallGameMethod<uintptr_t, 0x006a1be0>(&points->tree, iterator, &key, 0);
            }
        }
    } else {
        for (auto child : children) if (child) result = child->CollectCorners(points, rect);
    }
    return result;
}
// 0x00AF5ED0
uintptr_t TerrainQuad::EmitEdge(TerrainEdgeMesh* mesh, const TerrainRect* rect, TerrainEdgeOutput* output, uint8_t reverse) {
    auto points = EdgeSet(); points.tree.Clear();
    auto top = this;
    while (top->parent) top = top->parent;
    top->CollectCorners(&points, rect);
    auto sentinel = &points.tree.first;
    auto node = static_cast<TerrainEdgeNode*>(points.tree.last);
    uint32_t emitted = 0;
    while (node != reinterpret_cast<TerrainEdgeNode*>(sentinel)) {
        const uint32_t x = node->key & 127, y = node->key >> 7;
        const uint32_t row = x > 63 ? 63 : x, bit = y > 63 ? 63 : y;
        const float offset = mesh->bits->rows[row] & (uint64_t{1} << bit) ? -0.25f : 0.0f;
        CallGameMethod<void, 0x00af5d50>(mesh, x, y, offset, flags & 1);
        if (emitted) {
            const uint32_t index = output->count + emitted;
            CallGameMethod<void, 0x00af3d20>(this, mesh, output, 0, reverse ? index - 1 : index, reverse ? index : index - 1);
        }
        ++emitted;
        node = CallGameFunction<TerrainEdgeNode*, 0x004e0b20>(node);
    }
    output->count += emitted;
    return DestroyEdges(points, reinterpret_cast<uintptr_t>(output));
}
// 0x00AF6270. With an empty set EAX is a dead stack-sentinel address, never dereferenced by callers.
uintptr_t TerrainQuad::EmitFanEdge(TerrainMesh* mesh, const TerrainRect* rect, uint8_t reverse, const TerrainPoint* center) {
    auto points = EdgeSet(); points.tree.Clear();
    auto top = this;
    while (top->parent) top = top->parent;
    top->CollectCorners(&points, rect);
    auto sentinel = reinterpret_cast<TerrainEdgeNode*>(&points.tree.first);
    auto node = static_cast<TerrainEdgeNode*>(points.tree.last);
    TerrainPoint previous;
    bool first = true;
    while (node != sentinel) {
        auto height = mesh->source->heightfield;
        auto grid = height->grid;
        const uint32_t x = node->key & 127, y = node->key >> 7;
        const uint32_t index = y + grid->stride * (x + grid->width * (height->layer - height->minLayer));
        TerrainPoint current{static_cast<float>(x), grid->values[index], static_cast<float>(y)};
        if (!first) {
            AppendPoint(mesh->vertices, *center, true);
            AppendPoint(mesh->vertices, reverse ? previous : current, false);
            AppendPoint(mesh->vertices, reverse ? current : previous, false);
        }
        previous = current;
        first = false;
        node = CallGameFunction<TerrainEdgeNode*, 0x004e0b20>(node);
    }
    return DestroyEdges(points, reinterpret_cast<uintptr_t>(node));
}
// 0x00AF6510
uintptr_t TerrainQuad::EmitFans(TerrainMesh* mesh) {
    if (!leaf) {
        uintptr_t result = 0; // no defined EAX contract when every child is null.
        for (auto child : children) if (child) result = child->EmitFans(mesh);
        return result;
    }
    TerrainPoint center;
    CallGameMethod<void, 0x00af4f50>(this, &center, mesh, (bounds.maxX + bounds.minX) >> 1, (bounds.minY + bounds.maxY) >> 1);
    TerrainRect rect{bounds.minX, bounds.minY, bounds.maxX, bounds.minY};
    EmitFanEdge(mesh, &rect, 0, &center);
    rect = {bounds.minX, bounds.maxY, bounds.maxX, bounds.maxY};
    EmitFanEdge(mesh, &rect, 1, &center);
    rect = {bounds.minX, bounds.minY, bounds.minX, bounds.maxY};
    EmitFanEdge(mesh, &rect, 1, &center);
    rect = {bounds.maxX, bounds.minY, bounds.maxX, bounds.maxY};
    return EmitFanEdge(mesh, &rect, 0, &center);
}
namespace {
template<class T, unsigned Address> T& WorldGlobal() {
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(static_cast<uintptr_t>(Address));
#else
    NotRewritten("World game global");
#endif
}
void* GameAddress(unsigned address) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(static_cast<uintptr_t>(address));
#else
    (void)address; NotRewritten("World game callback/data address");
#endif
}
void ReleaseShutdownReference(ShutdownReference*& slot) {
    auto reference = slot;
    if (!reference) return;
    slot = nullptr;
    if (--reference->references == 0) {
        reference->references = 1;
        CallGameVirtual<void, 0>(reference, 1);
    }
}
void DetachSlot(void*& slot, void* manager, uint8_t notify) {
    DetachDatabase(slot, manager, notify);
    auto db = slot;
    if (db) {
        slot = nullptr;
        CallGameVirtual<void, 1>(static_cast<void**>(db) + 1);
    }
}
}
// 0x00C63ED0
void WorldShutdown::ClearReferences() {
    if (!references.size) return;
    void* registry = CallGameFunction<void*, 0x00938b50>();
    if (registry) {
        WorldReferenceIterator iterator;
        CallGameMethod<void, 0x00626610>(&references, &iterator);
        auto sentinel = references.buckets[references.count];
        while (iterator.node != sentinel) {
            auto node = iterator.node;
            auto record = CallGameMethod<void*, 0x00939560>(registry, node->keyLow, node->keyHigh);
            if (record && CallGameMethod<int32_t, 0x00935770>(record, WorldGlobal<void*, 0x011e8e90>()))
                CallGameMethod<void, 0x00939420>(registry, node->keyLow, node->keyHigh);
            iterator.node = node->next;
            while (!iterator.node) iterator.node = *++iterator.bucket;
        }
    }
    CallGameMethod<void, 0x007d3af0>(&references, references.buckets, references.count);
    references.size = 0;
}
// 0x00C6B780
void WorldShutdown::Shutdown() {
    if (owner.referenceA) {
        CallGameMethod<void, 0x00d680d0>(owner.referenceA);
        ReleaseShutdownReference(owner.referenceA);
    }
    ReleaseShutdownReference(owner.referenceB);
    ClearReferences();
    CallGameFunction<void, 0x0070ad80>();
    auto bus = CallGameFunction<void*, 0x0044c470>();
    if (bus) CallGameVirtual<void, 7>(bus, 59653228, nullptr, 0, nullptr);
    auto events = CallGameFunction<void*, 0x007692c0>();
    if (events) CallGameVirtual<void, 6>(events, 59653228, this, 0);
#if defined(_M_IX86)
    const auto currentThread = GetCurrentThreadId();
#else
    const auto currentThread = uint32_t{};
#endif
    if (currentThread == CallGameFunction<uint32_t, 0x007d99d0>()) {
        CallGameMethod<void, 0x00c6b500>(this);
    } else {
        void* storage = CallGameFunction<void*, 0x004e3f90>(32, "World/ShutdownFence", 0, 0, 0, 0);
        if (storage) {
            auto fence = CallGameMethod<void*, 0x00c60420>(storage, this);
            if (fence) {
                CallGameVirtual<void, 1>(fence);
                if (bus) {
                    auto bytes = static_cast<uint8_t*>(fence);
                    CallGameVirtual<void, 7>(bus, -705799079, fence, 0, bytes + 8);
                    CallGameMethod<void, 0x004e2760>(bytes + 16, GameAddress(0x010786c8));
                }
                CallGameVirtual<void, 2>(fence);
            }
        }
    }
    owner.auxiliary = nullptr;
    while (owner.lots.begin != owner.lots.end) owner.RemoveLot(owner.lots.end[-1], 1);
    if (owner.active) {
        CallGameVirtual<void, 5>(owner.active, 1);
        auto active = owner.active;
        if (active) { owner.active = nullptr; CallGameVirtual<void, 1>(active); }
    }
    auto manager = CallGameFunction<void*, 0x004afd20>();
    DetachSlot(owner.databaseA, manager, 1);
    DetachSlot(owner.databaseB, manager, 1);
    if (auto registry = CallGameFunction<void*, 0x00aab640>()) {
        WorldGlobal<uint32_t, 0x011843f4>() = 0;
        CallGameVirtual<void, 1>(registry, GameAddress(0x011843f0));
    }
    if (manager) CallGameVirtual<void, 9>(manager, 0, GameAddress(0x00c60690), this, 0, 0);
    if (auto keys = CallGameFunction<void*, 0x0072f1c0>()) {
        CallGameFunction<void, 0x00c649a0>(0, keys, GameAddress(0x00c60210));
        CallGameFunction<void, 0x00c649a0>(0, keys, GameAddress(0x00c60200));
    }
    DetachSlot(databaseC, manager, 0);
    DetachSlot(databaseD, manager, 0);
    if (auto pair = WorldGlobal<DatabasePair*, 0x011e96a4>()) pair->Cleanup();
    if (bus) CallGameVirtual<void, 11>(bus, this, 149881954, -9999);
    if (events) {
        CallGameVirtual<void, 11>(events, this, -60120578, -9999);
        CallGameVirtual<void, 11>(events, this, 236354871, -9999);
    }
    if (cleanup) {
        auto value = cleanup;
        value->Cleanup();
        CallGameFunction<void, 0x004e3f70>(value);
        cleanup = nullptr;
    }
    owner.loaded = owner.loading = owner.ready = 0;
}
namespace {
int8_t WallCoordinate(float value) {
    // fistp uses the current rounding mode, NOT the decompiler's truncating C cast.
    const double rounded = std::nearbyint(static_cast<double>(value));
    int32_t coordinate = std::numeric_limits<int32_t>::min();
    if (rounded >= -2147483648.0 && rounded < 2147483648.0) coordinate = static_cast<int32_t>(rounded);
    return static_cast<int8_t>(static_cast<uint8_t>(coordinate));
}
}
// 0x00D49710
uint8_t WallPlacement::Place(int32_t wall, int32_t direction, int32_t length, const WallPoint* start, WallPoint* end,
                             float height, uint8_t reverse, void* placement, void* context, void* existing) {
    // Checked data at 0x01088120: the eight compass directions, including diagonals.
    constexpr float directions[8][2] = {{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1},{0,1},{1,1}};
    const float dx = directions[direction][0], dy = directions[direction][1];
    float x = start->x, y = start->y;
    const int32_t layer = start->layer;
    if (end) {
        end->x = static_cast<float>(static_cast<float>(length) * dx) + x;
        end->y = static_cast<float>(static_cast<float>(length) * dy) + y;
        end->layer = layer;
    }
    auto locations = EdgeSet(); locations.name = "World/WallPlacementHelper/locSet"; locations.tree.Clear();
    for (int32_t i = 0; i < length; ++i) {
        const float nextX = x + dx, nextY = y + dy;
        WallSegment segment{WallCoordinate(x), WallCoordinate(y), static_cast<int8_t>(layer), WallCoordinate(nextX), WallCoordinate(nextY), static_cast<int8_t>(layer)};
        if (reverse) {
            auto a = segment.x1; segment.x1 = segment.x2; segment.x2 = a;
            a = segment.y1; segment.y1 = segment.y2; segment.y2 = a;
            a = segment.z1; segment.z1 = segment.z2; segment.z2 = a;
        }
        if (!CallGameMethod<uint8_t, 0x00d495d0>(this, &segment, wall, height, existing, placement, context)) {
            DestroyEdges(locations, 0); return 0;
        }
        if (!existing || !CallGameMethod<uint8_t, 0x00ad7d90>(existing)) {
            WallPoint first{static_cast<float>(segment.x1), static_cast<float>(segment.y1), segment.z1};
            WallPoint second{static_cast<float>(segment.x2), static_cast<float>(segment.y2), segment.z2};
            void* iterator[2];
            CallGameMethod<void, 0x00b0bad0>(&locations.tree, iterator, &first, 0);
            CallGameMethod<void, 0x00b0bad0>(&locations.tree, iterator, &second, 0);
        }
        x = nextX; y = nextY;
    }
    auto query = world->query; // even a nonpositive length reads this field in EA.
    if (placeObjects && CallGameFunction<uint8_t, 0x00c5bd30>(wall)) {
        x = start->x; y = start->y;
        const uint32_t spacing = CallGameFunction<uint32_t, 0x00c5c220>(wall);
        // EA's length<=0 guard is inside the loop: do not emit the nominal endpoint then.
        if (length > 0) for (int32_t i = 0; i <= length; ++i) {
            WallPoint point{x,y,layer};
            void* iterator[2];
            auto found = CallGameMethod<void**, 0x00b05530>(&locations.tree, iterator, &point);
            if (*found != &locations.tree.first &&
                (!i || i == length || CallGameFunction<uint8_t, 0x00c5ba20>(world, &point, 0, 0) || (spacing && static_cast<uint32_t>(i) % spacing == 0))) {
                if (CallGameMethod<uint8_t, 0x00b48670>(query, &point)) {
                    if (CallGameMethod<uint8_t, 0x00d46a60>(this, &point))
                        CallGameMethod<void, 0x00d2e130>(placement, world, &point);
                } else {
                    uint8_t settingsCopy[48];
                    auto settingsValue = CallGameMethod<void*, 0x00afd5d0>(settingsCopy, settings[1], settings[2], settings[0]);
                    CallGameMethod<void, 0x00d2df30>(placement, world, &point, wall, settingsValue);
                }
            }
            x += dx; y += dy;
        }
    }
    DestroyEdges(locations, 0);
    return 1;
}
// 0x00D44D50, not the preceding 0x00D44D30 prologue/body.
// EBX=0 is established at D44D34; the enclosing saved-register frame is handled by the caller adapter.
uintptr_t LotOperationTail::Finish(void* active) {
    if (active) {
        auto lot = lotId < 0 ? CallGameMethod<void*, 0x00d43360>(this) : CallGameMethod<void*, 0x00c60e10>(active, lotId);
        if (lot) {
            CallGameMethod<void, 0x00c6e3b0>(lot, 1, 0);
            locations.Clear();
            pending = 0;
        }
    }
    return CallGameMethod<uintptr_t, 0x00ccead0>(this);
}
}
