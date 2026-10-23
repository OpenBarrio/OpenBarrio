#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::world {
struct TreeNode { TreeNode* child; TreeNode* sibling; };
TS3_X86_OFFSET(TreeNode, sibling, 4);
TS3_X86_SIZE(TreeNode, 8);
struct WorldTree {
    uint32_t unknown;
    void* first;
    void* last;
    TreeNode* root;
    uint8_t flag;
    uint8_t padding[3];
    uint32_t count;
    // TS3W.exe 0x00B40CD0: free nodes and reset the intrusive sentinel, returning zero.
    int32_t Clear();
};
TS3_X86_OFFSET(WorldTree, first, 4);
TS3_X86_OFFSET(WorldTree, root, 12);
TS3_X86_OFFSET(WorldTree, flag, 16);
TS3_X86_OFFSET(WorldTree, count, 20);
TS3_X86_SIZE(WorldTree, 24);
// TS3W.exe 0x006A0850: post-order free child/sibling nodes; null has no defined EAX result.
void FreeTree(TreeNode* node);
struct KeyList { void* begin; void* end; void* capacity; const char* name; };
TS3_X86_SIZE(KeyList, 16);
// TS3W.exe 0x007D8E50: detach a database, optionally notify its keys, then close and release it.
void DetachDatabase(void* database, void* manager, uint8_t notify);
struct WorldCleanup {
    uint8_t prefix[516];
    uint8_t embedded[592];
    TreeNode* root;
    // TS3W.exe 0x004A47A0: clean the world, free its tree, and finalize its embedded object.
    int32_t Cleanup();
};
TS3_X86_OFFSET(WorldCleanup, embedded, 516);
TS3_X86_OFFSET(WorldCleanup, root, 1108);
struct DatabasePair {
    uint8_t prefix[120];
    void* first;
    void* second;
    // TS3W.exe 0x009A2FE0: detach/release the second database before the first.
    void Cleanup();
};
TS3_X86_OFFSET(DatabasePair, first, 120);
TS3_X86_OFFSET(DatabasePair, second, 124);
struct ShortRange { const int16_t* begin; const int16_t* end; };
struct WorldSelections {
    uint8_t prefix[36];
    void** cursor;
    void** unused;
    void** pageEnd;
    void*** page;
    void** finish;
    uint8_t middle[28];
    WorldTree selections;
    uint32_t gap1;
    WorldTree exclusions;
    uint32_t gap2;
    WorldTree finalSelections;
    // TS3W.exe 0x00ADC810: replace selections and propagate them to each deque item.
    void* SetSelections(ShortRange* range);
    // TS3W.exe 0x00ADC8C0: replace exclusions and propagate them to each deque item.
    void* SetExclusions(ShortRange* range);
    // TS3W.exe 0x00ADC970: replace the final signed-short selection set.
    void* SetFinalSelections(ShortRange* range);
};
TS3_X86_OFFSET(WorldSelections, cursor, 36);
TS3_X86_OFFSET(WorldSelections, finish, 52);
TS3_X86_OFFSET(WorldSelections, selections, 84);
TS3_X86_OFFSET(WorldSelections, exclusions, 112);
TS3_X86_OFFSET(WorldSelections, finalSelections, 140);
struct Candidate {
    uint8_t prefix[16];
    int32_t kind;
    uint8_t middle[8];
    int32_t key;
    uint8_t gap[4];
    void* validator;
    Candidate* related;
    // TS3W.exe 0x006C8080: validate a candidate and insert its related key if present.
    uint8_t Collect(WorldTree* output);
};
TS3_X86_OFFSET(Candidate, key, 28);
TS3_X86_OFFSET(Candidate, validator, 36);
TS3_X86_OFFSET(Candidate, related, 40);
struct CandidateRange { Candidate** begin; Candidate** end; };
struct CandidateBucket {
    uint8_t prefix[8];
    CandidateRange* range;
    uintptr_t unknown;
    CandidateBucket* next;
};
TS3_X86_OFFSET(CandidateBucket, next, 16);
struct CandidateTable {
    uint8_t prefix[220];
    CandidateBucket** buckets;
    uint32_t count;
    // TS3W.exe 0x006C6D40: reset output, then collect matching kind-2 candidates.
    uint8_t Collect(int32_t key, WorldTree* output);
};
TS3_X86_OFFSET(CandidateTable, buckets, 220);
TS3_X86_OFFSET(CandidateTable, count, 224);
struct WorldLot {
    void** vtable;
    uint8_t prefix[12];
    void* parent;
    uint8_t middle[52];
    uint32_t idLow, idHigh;
};
TS3_X86_OFFSET(WorldLot, parent, 16);
TS3_X86_OFFSET(WorldLot, idLow, 72);
struct WorldLotPointerVector { WorldLot** begin; WorldLot** end; WorldLot** capacity; };
struct ShutdownReference { void** vtable; uint32_t references; };
struct WorldLotOwner {
    uint8_t prefix[65];
    uint8_t ready;
    uint8_t flagPadding[2];
    uint8_t loaded, loading;
    uint8_t preReferences[50];
    ShutdownReference* referenceA;
    ShutdownReference* referenceB;
    void* databaseA;
    void* databaseB;
    uint8_t preLots[20];
    WorldLotPointerVector lots;
    uint32_t unused168;
    void* auxiliary;
    WorldLot* active;
    // TS3W.exe 0x00C65A20: remove a non-active lot, forwarding to its parent if owned.
    void RemoveLot(WorldLot* lot, int32_t destroy);
};
TS3_X86_OFFSET(WorldLotOwner, lots, 156);
TS3_X86_OFFSET(WorldLotOwner, active, 176);
struct WorldMemoryDatabase {
    uint8_t prefix[132];
    void* database;
    // TS3W.exe 0x00C604B0: replace and initialize the world's memory database.
    uint8_t Create();
};
TS3_X86_OFFSET(WorldMemoryDatabase, database, 132);
struct TerrainRect { uint32_t minX, minY, maxX, maxY; };
struct TerrainPoint { float x, y, z; };
TS3_X86_SIZE(TerrainPoint, 12);
struct TerrainPointVector { TerrainPoint* begin; TerrainPoint* end; TerrainPoint* capacity; };
struct TerrainGrid { uint32_t unused, width, stride, unused2; float* values; };
struct TerrainHeightfield {
    uint8_t prefix[16];
    uint32_t minLayer, unused, layer;
    uint8_t middle[8];
    TerrainGrid* grid;
};
TS3_X86_OFFSET(TerrainHeightfield, grid, 36);
struct TerrainSource { uint8_t prefix[656]; TerrainHeightfield* heightfield; };
struct TerrainMesh { TerrainSource* source; TerrainPointVector vertices; };
struct TerrainBits { const uint64_t* rows; };
struct TerrainEdgeMesh { uint8_t prefix[8]; TerrainBits* bits; };
struct TerrainEdgeOutput { uint8_t prefix[12]; uint32_t count; };
struct TerrainEdgeSet { WorldTree tree; const char* name; };
TS3_X86_SIZE(TerrainEdgeSet, 28);
struct TerrainEdgeNode {
    TerrainEdgeNode* child;
    TerrainEdgeNode* sibling;
    TerrainEdgeNode* prev;
    TerrainEdgeNode* next;
    uint32_t key;
};
TS3_X86_OFFSET(TerrainEdgeNode, key, 16);
struct TerrainQuad {
    TerrainRect bounds;
    uint8_t leaf;
    uint8_t padding[3];
    uint32_t flags;
    TerrainQuad* children[4];
    TerrainQuad* parent;
    // TS3W.exe 0x00AF5BD0: collect overlapping leaf corners into the ordered point set.
    uintptr_t CollectCorners(TerrainEdgeSet* points, const TerrainRect* rect);
    // TS3W.exe 0x00AF5ED0: emit perimeter vertices/indices, using clamped occupancy samples.
    uintptr_t EmitEdge(TerrainEdgeMesh* mesh, const TerrainRect* rect, TerrainEdgeOutput* output, uint8_t reverse);
    // TS3W.exe 0x00AF6270: emit a triangle fan for one clipped edge, sampling terrain heights.
    uintptr_t EmitFanEdge(TerrainMesh* mesh, const TerrainRect* rect, uint8_t reverse, const TerrainPoint* center);
    // TS3W.exe 0x00AF6510: recursively tessellate all four edges of every leaf.
    uintptr_t EmitFans(TerrainMesh* mesh);
};
TS3_X86_OFFSET(TerrainQuad, leaf, 16);
TS3_X86_OFFSET(TerrainQuad, children, 24);
TS3_X86_OFFSET(TerrainQuad, parent, 40);
TS3_X86_SIZE(TerrainQuad, 44);
struct WorldReferenceNode { uint32_t keyLow, keyHigh; WorldReferenceNode* next; };
struct WorldReferenceIterator { WorldReferenceNode* node; WorldReferenceNode** bucket; };
struct WorldReferenceSet {
    uint32_t unused;
    WorldReferenceNode** buckets;
    uint32_t count, size;
};
struct WorldShutdown {
    WorldLotOwner owner;
    uint8_t middle[764];
    WorldCleanup* cleanup;
    void* databaseC;
    void* databaseD;
    WorldReferenceSet references;
    // TS3W.exe 0x00C63ED0: remove matching registered references, then clear the hash table.
    void ClearReferences();
    // TS3W.exe 0x00C6B780: tear down world references, lots, databases and event subscriptions.
    void Shutdown();
};
TS3_X86_OFFSET(WorldLotOwner, ready, 65);
TS3_X86_OFFSET(WorldLotOwner, loaded, 68);
TS3_X86_OFFSET(WorldLotOwner, referenceA, 120);
TS3_X86_OFFSET(WorldLotOwner, databaseA, 128);
TS3_X86_OFFSET(WorldLotOwner, auxiliary, 172);
TS3_X86_OFFSET(WorldShutdown, cleanup, 944);
TS3_X86_OFFSET(WorldShutdown, references, 956);
TS3_X86_SIZE(WorldShutdown, 972);
struct WallPoint { float x, y; int32_t layer; };
struct WallSegment { int8_t x1,y1,z1,x2,y2,z2; };
TS3_X86_SIZE(WallSegment, 6);
struct WallWorld { uint8_t prefix[664]; void* query; };
struct WallPlacement {
    uint8_t prefix[8];
    WallWorld* world;
    uint8_t middle[28];
    uint8_t settings[3][16];
    uint8_t placeObjects;
    // TS3W.exe 0x00D49710: validate/place wall segments and optional endpoint objects.
    uint8_t Place(int32_t wall, int32_t direction, int32_t length, const WallPoint* start, WallPoint* end,
                  float height, uint8_t reverse, void* placement, void* context, void* existing);
};
TS3_X86_OFFSET(WallPlacement, world, 8);
TS3_X86_OFFSET(WallPlacement, settings, 40);
TS3_X86_OFFSET(WallPlacement, placeObjects, 88);
struct LotOperationTail {
    uint8_t prefix[196]; uint8_t pending; uint8_t middle[31];
    WorldTree locations; uint8_t tail[40]; int32_t lotId;
    // TS3W.exe 0x00D44D50: internal tail entered with EDI=this, ECX=active, EBX=0 and saved EDI/EBX on stack.
    // The portable adapter takes active explicitly; EXEHOST seeds that exact original register/frame ABI.
    uintptr_t Finish(void* active);
};
TS3_X86_OFFSET(LotOperationTail,pending,196);
TS3_X86_OFFSET(LotOperationTail,locations,228);
TS3_X86_OFFSET(LotOperationTail,lotId,292);
}
