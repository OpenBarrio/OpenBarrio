#pragma once
#include "ts3/layout.h"
#include <cstdint>

namespace ts3::renderer {
struct RenderArray {
    void* data;
    uint32_t size, capacity;
    const char* name;
    // TS3W.exe 0x006E0540: initialize a render-set array, leaving no storage allocated.
    RenderArray* InitRenderSet();
};
TS3_X86_OFFSET(RenderArray, name, 12);
TS3_X86_SIZE(RenderArray, 16);
struct DisplayList {
    uint8_t untouched[32];
    RenderArray bins[15], sets[15], activeMask, transforms;
    // TS3W.exe 0x0068E540: initialize the display-list arrays; the leading 32 bytes are untouched.
    DisplayList* Init();
};
TS3_X86_OFFSET(DisplayList, sets, 272);
TS3_X86_OFFSET(DisplayList, activeMask, 512);
TS3_X86_SIZE(DisplayList, 544);
struct Scene;
struct LodSchedulerLink {
    uint8_t prefix[32];
    Scene* scene;
    // TS3W.exe 0x006E7550: set the scheduler's scene pointer and return true.
    uint8_t SetScene(Scene* value);
};
TS3_X86_OFFSET(LodSchedulerLink, scene, 32);
struct alignas(16) CameraVector { uint32_t bits[4]; };
struct CameraParameters {
    uint8_t prefix[64];
    CameraVector vectors[3];
    float nearPlane, farPlane, limit;
    // TS3W.exe 0x0096BA10: copy three vectors and three scalars, returning the far-plane output pointer.
    float* Get(CameraVector* a, CameraVector* b, CameraVector* c, float* nearOut, float* farOut, float* limitOut);
};
TS3_X86_OFFSET(CameraParameters, vectors, 64);
TS3_X86_OFFSET(CameraParameters, limit, 120);
struct SceneLink { SceneLink* next; SceneLink* prev; };
struct SceneNode {
    void** vtable;
    uint32_t refs;
    SceneLink sibling;
    uint32_t unknown16;
    SceneNode* parent;
    SceneLink children;
    uint8_t tail[56];
};
TS3_X86_OFFSET(SceneNode, sibling, 8);
TS3_X86_OFFSET(SceneNode, parent, 20);
TS3_X86_OFFSET(SceneNode, children, 24);
TS3_X86_SIZE(SceneNode, 88);
struct Scene {
    SceneNode* root;
    RenderArray objects;
    uint32_t unknown20, unknown24;
    void* owner;
    SceneLink links;
    int32_t lock;
    uint32_t unknown44;
    RenderArray frustum, visible;
    uint32_t unknown80, unknown84, unknown88, unknown92;
    uint8_t enabled, padding[3];
    uint32_t unknown100;
    // TS3W.exe 0x006E3FA0: allocate and attach a node, defaulting a null parent to the scene root.
    uint8_t CreateNode(SceneNode* parent, SceneNode** output);
    // TS3W.exe 0x006E4530: initialize the scene and attempt to create its root node.
    Scene* Init(void* ownerValue);
};
TS3_X86_OFFSET(Scene, objects, 4);
TS3_X86_OFFSET(Scene, links, 32);
TS3_X86_OFFSET(Scene, frustum, 48);
TS3_X86_OFFSET(Scene, enabled, 96);
TS3_X86_SIZE(Scene, 104);
struct Camera;
struct LodScheduler;
struct RendererSceneHost {
    void* unknown0;
    void* unknown4;
    Scene* scene;
    LodScheduler* scheduler;
    DisplayList* lists[5];
    Camera* cameras[5];
    uint8_t config[612]; // Original +56 through +667: scalar settings and untouched storage.
    // TS3W.exe 0x006E9270: reload renderer configuration and return the decremented mode selector.
    int32_t ReloadConfig();
    // TS3W.exe 0x006EB5A0: lazily allocate a camera and acquire its first reference.
    uint8_t EnsureCamera(Camera** output);
    // TS3W.exe 0x006EB610: allocate scene, scheduler, five display lists and five cameras in order.
    uint8_t Initialize();
    // TS3W.exe 0x006E8480: create a node through the scene, or return false without touching output.
    uint8_t CreateNode(SceneNode* parent, SceneNode** output);
    // TS3W.exe 0x006EB540: lazily allocate a display list, cleaning up the host on allocation failure.
    uint8_t EnsureDisplayList(DisplayList** output);
};
TS3_X86_OFFSET(RendererSceneHost, scene, 8);

TS3_X86_OFFSET(RendererSceneHost, config, 56);
// TS3W.exe 0x0058AAD0: case-insensitive boolean configuration lookup with the override table taking precedence.
uint8_t ConfigBool(const wchar_t* section, const wchar_t* key, uint8_t fallback);
struct LodScheduler {
    uint32_t limits[5], untouched20;
    uint8_t flags[4];
    uint32_t unknown28;
    Scene* scene;
    uint32_t unknown36;
    float factor, adjust;
    uint32_t unknown48[2];
    float distances[5], squared[5], nearDistance, farDistance;
    RenderArray activeMask, objectStates;
    // TS3W.exe 0x006E7850: initialize LOD scheduling defaults, read overrides and update the global cull factor.
    LodScheduler* Init();
};
TS3_X86_OFFSET(LodScheduler, scene, 32);
TS3_X86_OFFSET(LodScheduler, distances, 56);
TS3_X86_OFFSET(LodScheduler, activeMask, 104);
TS3_X86_SIZE(LodScheduler, 136);
struct Frustum {
    float vectors[44][4];
    // TS3W.exe 0x0068D8A0: initialize planes/bounds and ask the game to build derived plane vectors.
    void Init();
};
TS3_X86_SIZE(Frustum, 704);
struct Camera {
    void** vtable;
    int32_t refs;
    uint8_t body[1760]; // Scalar/vector storage through original +1767.
    void* reserved[4]; // Zero-initialized cells: conservatively pointer-wide until their consumers are recovered.
    int32_t index;
    uint8_t tail[52];
    // TS3W.exe 0x00962F80: initialize a camera, its frustum, parameters and view transform.
    Camera* Init();
    // Compare-engine storage is only byte-aligned; use an aligned temporary for the game's MOVAPS bridges.
    Camera* InitForCompare();
};
TS3_X86_OFFSET(Camera, body, 8);
TS3_X86_OFFSET(Camera, reserved, 1768);
TS3_X86_OFFSET(Camera, index, 1784);
TS3_X86_OFFSET(Camera, tail, 1788);
TS3_X86_SIZE(Camera, 1840);
struct TreeLodConfig {
    uint8_t prefix[20]; uint8_t alphaEdges; uint8_t gap[355];
    float ranges[4][5];
    // TS3W.exe 0x00713430: load the four tree LOD ranges, inverse fade widths and alpha-edge flag.
    uint8_t Reload();
};
TS3_X86_OFFSET(TreeLodConfig, ranges, 376);
struct MinimizedConfig {
    uint8_t prefix[44]; uint8_t simulate;
    // TS3W.exe 0x0076B3C0: reload SimWhileMinimized and return the byte stored.
    uint8_t Reload();
};
struct RotationWeights {
    uint8_t chains[4][256];
    // TS3W.exe 0x0097C2F0: set vertical inversion weights on each of the four chains.
    int SetVertical(uint8_t value);
};
struct RotationConfig {
    uint8_t prefix[148]; void* horizontal;
    uint8_t gap152[8]; void* secondary;
    uint8_t gap164[4]; RotationWeights* vertical;
    uint8_t gap172[53]; uint8_t invertX, invertY, squarePixels;
    // TS3W.exe 0x0096B730: reload inversion, edge scrolling and square-pixel settings and update both chain sets.
    uint8_t Reload();
};
TS3_X86_OFFSET(RotationConfig, invertX, 225);
struct CullGroup {
    uint8_t prefix[28]; void* queryOwner;
    uint8_t gap32[252]; uint8_t enabled; uint8_t padding[3]; int32_t mask;
    void** firstBegin; void** firstEnd; uint8_t gap300[8]; void** secondBegin; void** secondEnd;
    // TS3W.exe 0x00B25D10: change the cull mask and detach/reattach both object lists; return only AL's last value.
    uint8_t SetCull(uint8_t value);
};
TS3_X86_OFFSET(CullGroup, enabled, 284);
TS3_X86_OFFSET(CullGroup, secondBegin, 308);
struct ObjectHidingConfig {
    uint8_t prefix[49]; uint8_t cull;
    uint8_t gap50[546]; CullGroup* group;
    uint8_t gap600[256]; uint8_t hiding;
    uint8_t gap857[61]; uint8_t unload;
    // TS3W.exe 0x00AE3870: reload hiding/culling settings, propagate a changed flag and refresh visibility.
    int Reload();
};
TS3_X86_OFFSET(ObjectHidingConfig, group, 596);
TS3_X86_OFFSET(ObjectHidingConfig, unload, 918);
struct UndergroundObject {
    uint8_t prefix[850]; uint8_t cull;
    // TS3W.exe 0x00C0E140: reload the underground-culling flag on an individual object.
    uint8_t Reload();
};
struct UndergroundConfig {
    uint8_t prefix[129]; uint8_t cull; uint8_t gap130[582];
    UndergroundObject** begin; UndergroundObject** end;
    // TS3W.exe 0x00C0B4C0: reload each child object's underground-culling flag, then the parent's.
    uint8_t Reload();
};
TS3_X86_OFFSET(UndergroundConfig, begin, 712);
// TS3W.exe 0x00EC5480: return zero when muted, otherwise bitwise-clamped normalized music level times movie gain.
double MovieVolume(const wchar_t* movie);
}
