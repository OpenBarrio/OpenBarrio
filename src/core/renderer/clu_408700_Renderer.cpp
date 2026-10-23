#include "ts3/renderer/clu_408700_Renderer.h"
#include "ts3/routing/clu_5a00a0_Routing.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <intrin.h>
#include <cstdlib>

namespace ts3::renderer {
namespace {
template<unsigned Address> void* GameGlobal()
{
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(static_cast<uintptr_t>(Address));
#else
    AbortNotRewritten("renderer game global");
    return nullptr;
#endif
}
template<unsigned Address> void* GameAddress()
{
#if defined(_M_IX86)
    return reinterpret_cast<void*>(static_cast<uintptr_t>(Address));
#else
    AbortNotRewritten("renderer game address");
    return nullptr;
#endif
}
struct ParameterEntry { uint32_t version; uint16_t generation, padding; uint32_t reserved; void* resource; };
struct ParameterTable { void* vtable; ParameterEntry* entries; uint32_t count; uint8_t reserved[6]; uint16_t generation; };
TS3_X86_OFFSET(ParameterEntry, resource, 12);
TS3_X86_SIZE(ParameterEntry, 16);
TS3_X86_OFFSET(ParameterTable, generation, 18);
}

uint32_t StartTimer408700(RenderTimer* self)
{
    // 0x00408700 returns OR of both halves when already started.
    const uint32_t running = self->low | self->high;
    if (running) return running;
    if (self->clock == 1) {
        const uint64_t value = __rdtsc();
        self->low = static_cast<uint32_t>(value);
        self->high = static_cast<uint32_t>(value >> 32);
        return self->low;
    }
    LARGE_INTEGER value;
    QueryPerformanceCounter(&value);
    self->low = value.LowPart;
    self->high = static_cast<uint32_t>(value.HighPart);
    return self->high;
}

bool LoadProxy722F50(ResourceProxy* self, const void* key, bool wait, int priority, int flags)
{
    // 0x00722F50: callbacks may change the global manager; reload it for the wait.
    auto* manager = GameGlobal<0x11D3C84>();
    if (!manager) return false;
    if (!CallGameVirtual<bool, 13>(manager, key, GameAddress<0xB3D7A7>(), &self->resource,
                                  priority, 0, GameAddress<0xD4B150>(), flags, 1)) return false;
    if (!wait) return true;
    manager = GameGlobal<0x11D3C84>();
    if (manager) CallGameVirtual<void, 16>(manager, &self->resource);
    return self->resource != 0 && (self->resource & 3) == 0;
}

uintptr_t ReleaseProxy722FC0(ResourceProxy* self)
{
    // 0x00722FC0: pending tagged handles deliberately remain in the proxy.
    if (auto* manager = GameGlobal<0x11D3C84>())
        CallGameVirtual<void, 15>(manager, &self->resource, GameAddress<0xD4B150>());
    uintptr_t result = self->resource;
    if (result && !(result & 3)) {
        result = CallGameVirtual<uintptr_t, 1>(reinterpret_cast<void*>(result));
        self->resource = 0;
    }
    return result;
}

ResourceProxy* ConstructProxy723020(ResourceProxy* self, const void* key, bool wait)
{
    // 0x00723020 leaves the allocation's final dword untouched.
    for (auto& word : self->key) word = 0;
    self->resource = 0;
    LoadProxy722F50(self, key, wait, 0, 0);
    return self;
}

bool BindTextureC88BF0(ResourceProxy* self, const char* name, const wchar_t* path, uint32_t* parameter)
{
    // 0x00C88BF0 preserves the output parameter if key construction fails.
    uint32_t key[4]{};
    ReleaseProxy722FC0(self);
    if (!CallGameFunction<bool, 0x4AECE0>(key, path, GameAddress<0xB2D882>(), 0, 0, 0)) return false;
    auto* table = static_cast<ParameterTable*>(GameGlobal<0x11D7530>());
    *parameter = CallGameMethod<uint32_t, 0x79A160>(table, name, 1, 5);
    const bool loaded = LoadProxy722F50(self, key, true, 0, 0);
    void* resource = loaded ? CallGameMethod<void*, 0x722F30>(self) : nullptr;
    if (*parameter < table->count) {
        auto& entry = table->entries[*parameter];
        ++entry.version;
        entry.generation = table->generation;
        entry.resource = resource;
    }
    return loaded;
}

bool BindTextureC88FB0(ResourceProxy* self, const char* name, const wchar_t* path)
{
    // 0x00C88FB0: the initial scratch dword is dead when key construction fails.
    uint32_t parameter;
    return BindTextureC88BF0(self, name, path, &parameter);
}
// Original game global references, accessed only by routines that bridge to game code.
template<class T> T& SceneGlobal(uintptr_t ea) {
#if defined(_M_IX86)
 return *reinterpret_cast<T*>(ea);
#else
 (void)ea;
 NotRewritten("renderer game global");
#endif
}
static void* AllocateScene(size_t size,const char* name) {
 return CallGameFunction<void*,0x4e3f90>(size,name,0,0,0,0);
}
static void* AllocateSceneAligned(size_t size,uint32_t alignment,const char* name) {
 return CallGameFunction<void*,0x4e3fb0>(size,alignment,0,name,0,0,0,0);
}
// 006DEEE0: thiscall, ret 8.
bool PickManager::Initialize(void* a2,void* a3) {
 void* allocation=AllocateScene(544,"Renderer/RenderPickManager/DisplayList");
 p8=allocation?CallGameMethod<void*,0x68e540>(allocation):nullptr;
 if(!p8) return false;
 p4=a3; p0=a2;
 void* table=GameGlobal<0x11d7530>();
 u12=CallGameMethod<uint32_t,0x79a160>(table,"renderPickId",1,2);
 u16=CallGameMethod<uint32_t,0x79a160>(table,"renderPickTransform",1,2);
 return true;
}
// 006DEF60: thiscall, ret 0. Lookup provider independently each time.
int PickManager::RegisterOptions() {
 void* options=ts3::routing::GetTuningObject();
 if(options) CallGameVirtual<void,6>(options,L"Rendering",4000);
 options=ts3::routing::GetTuningObject();
 if(options) CallGameVirtual<void,10>(options,&b22,L"RenderPick FrameDelay");
 options=ts3::routing::GetTuningObject();
 return options?CallGameVirtual<int,6>(options,static_cast<const wchar_t*>(nullptr),0x7fffffff):0;
}
// 006DF0E0: thiscall, ret 0; many bytes intentionally remain uninitialized.
PickManager* PickManager::Construct() {
 p0=p4=p8=nullptr; b20=b21=b22=0;
 u24=1; u28=0x7ffff000; u32=u36=1;
 f40=f44=f48=f52=1.0f;
 for(auto& id:ids56) id=0xffffffffu;
 u96=u100=0; b256=b257=0; b258=1; b259=b260=0;
 b22=static_cast<uint8_t>(CallGameFunction<int,0x58aad0>(GameGlobal<0x114fab0>(),L"RenderPickDelayReadback",0));
 return this;
}
// Two game vectors: each stores begin/end/capacity and an allocator pointer.
struct MaterialBuilder {
 struct Vector { void* begin; void* end; void* capacity; void* allocator; } vectors[2];
};
TS3_X86_SIZE(MaterialBuilder,32);
// 006ED2B0: thiscall, ret 0.
bool SceneManager::InitializePerFrameMaterial() {
 if(p804) return true;
 void* allocation=AllocateSceneAligned(64,8,"Renderer/SceneManager/PerFrameMaterial");
 if(!allocation) return false;
 void* material=CallGameMethod<void*,0x6caf80>(allocation);
 if(!material) return false;
 CallGameVirtual<void,0>(material);
 MaterialBuilder builder;
 CallGameMethod<void,0x799300>(&builder);
 const char* names[]={"TimeData","GameTimeData","TerrainCenterPos","TerrainGeomorphDistances","LightingTweaks","ProbeGenerationTweaks","SimLightingTweaks1","SimLightingTweaks2","SimLightingTweaks3"};
 for(const char* name:names) CallGameMethod<void,0x799700>(&builder,name,static_cast<void*>(nullptr),4);
 CallGameMethod<void,0x799700>(&builder,"OpaqueWindowDistance",static_cast<void*>(nullptr),2);
 CallGameMethod<void,0x799700>(&builder,"PlatformMipFilter",&u440,1);
 CallGameMethod<void,0x799700>(&builder,"PlatformMinFilter",&u444,1);
 float joints[60][12]{};
 for(auto& joint:joints) joint[0]=joint[5]=joint[10]=1.0f;
 CallGameMethod<void,0x799700>(&builder,"jointMatrix",joints,720);
 bool success=false;
 if(CallGameMethod<uint8_t,0x6cae90>(material,&builder)) {
  int type=CallGameFunction<int,0x6cacd0>();
  success=CallGameMethod<uint8_t,0x6cac30>(material,&p804,type)!=0;
 }
 CallGameMethod<void,0x799330>(&builder);
 CallGameVirtual<void,1>(material);
 return success;
}
// Game-owned objects touched by Initialize. Their pointer slots grow normally on x64.
struct SceneRendererTarget { std::byte unknown0[12]; void* p12; };
struct SceneRendererGlobal { std::byte unknown0[244]; SceneRendererTarget* p244; std::byte unknown248[8]; void* p256; };
struct SceneCallback { void* vtable; uint32_t unknown4; void* callback; SceneManager* context; };
struct SceneConfig { std::byte unknown0[280]; const char* text; };
TS3_X86_OFFSET(SceneCallback,callback,8); TS3_X86_OFFSET(SceneCallback,context,12);
TS3_X86_OFFSET(SceneRendererGlobal,p244,244); TS3_X86_OFFSET(SceneRendererGlobal,p256,256);
TS3_X86_OFFSET(SceneConfig,text,280);
// 006F40B0: thiscall, ret 0.
bool SceneManager::Initialize() {
 if(!CallGameMethod<uint8_t,0x6eb610>(this)) return false;
 auto fail=[&]() { CallGameMethod<void,0x6e97f0>(this); return false; };
 auto* pick=static_cast<PickManager*>(AllocateScene(sizeof(PickManager),"Renderer/RenderPickManager"));
 p56=pick?pick->Construct():nullptr;
 if(!p56 || !p56->Initialize(p36,p8)) return fail();
 if(this==GameGlobal<0x11d1868>()) p56->RegisterOptions();
 ParameterTable* table=nullptr;
 auto param=[&](const char* name,int count,int type) {return CallGameMethod<uint32_t,0x79a160>(table,name,count,type);};
 auto material=[&](const char* name,int flags,int required) {return CallGameMethod<void*,0x79b840>(GameGlobal<0x11d753c>(),name,flags,required);};
 p740=material("shadowmap",0,1);
 auto* render=static_cast<SceneRendererGlobal*>(GameGlobal<0x11d1530>());
 void* mergedTarget=render->p244->p12;
 table=static_cast<ParameterTable*>(GameGlobal<0x11d7530>());
 p748=CallGameFunction<void*,0x70bbf0>("shadowmapmerged",mergedTarget,0);
 render=static_cast<SceneRendererGlobal*>(GameGlobal<0x11d1530>());
 p752=CallGameFunction<void*,0x70bbf0>("dropshadow",render->p256,0);
 u756=param("DropShadowAtlas2",1,5);
 p768=material("PostProcess",0,0); p772=material("RainRipples",0,0);
 if(p768) {
  u1340=CallGameMethod<uint32_t,0x79a550>(p768,"Bloom");
  u1344=CallGameMethod<uint32_t,0x79a550>(p768,"HeatShimmer");
  u1348=CallGameMethod<uint32_t,0x79a550>(p768,"UnderSea");
 }
 p776=material("CASLightMap",0,0);
 auto shader=[&](unsigned offset,const char* name,int count,int type) { shader464[(offset-464)/4]=param(name,count,type); };
 shader(464,"WorldToClipSpaceMatrix",4,2); shader(468,"ObjectToWorldSpaceMatrix",4,2);
 shader(472,"ObjectToClipSpaceMatrix",4,2); shader(476,"InstacedModelOffsetMatrix",4,2);
 shader(480,"DisplayToWorldSpaceMatrix",4,2); shader(508,"ShadowTextureData",4,2);
 shader(512,"LocalToLightSpaceMatrix",4,2); shader(516,"WorldToLightSpaceMatrix",4,2);
 shader(520,"WorldSpaceLightPosition",4,2); shader(524,"PrecipitationShadowTextureData",4,2);
 shader(528,"LocalToPrecipitationShadowMatrix",4,2); shader(532,"WorldToPrecipitationShadowMatrix",4,2);
 shader(536,"WorldSpacePrecipitationShadowCameraPosition",4,2); shader(560,"DoShadowLookup",1,4);
 u960=param("VisualizerControl",4,2);
 shader(548,"ShadowMainMap",1,5); shader(484,"ReflectionParams",1,2);
 shader(556,"PrecipitationShadowMainMap",1,5); shader(552,"WaterReflectionMap",1,5);
 shader(488,"SimBaseHeight",1,1); shader(492,"RainRippleUVRandomOffset",2,1); shader(564,"SeaMurkScale",1,1);
 if(!InitializePerFrameMaterial()) return fail();
 StartTimer408700(&timer568);
 if(void* old=p448) {CallGameMethod<void,0x6b0170>(old); CallGameFunction<void,0x4e3f70>(old);}
 void* allocation=AllocateSceneAligned(400,16,"Renderer/LightManager");
 p448=allocation?CallGameMethod<void*,0x6afdc0>(allocation,this):nullptr;
 // Release in source order; retain the original lack of allocation-result checks below.
 if(p452) {CallGameVirtual<void,0>(p452,1);p452=nullptr;}
 if(p456) {CallGameVirtual<void,0>(p456,1);p456=nullptr;}
 allocation=AllocateSceneAligned(68,4,"Renderer/InteriorShadowManager");
 p452=allocation?CallGameMethod<void*,0x6998b0>(allocation):nullptr;
 allocation=AllocateSceneAligned(48,4,"Renderer/InteriorShadowManager");
 p456=allocation?CallGameMethod<void*,0x692790>(allocation):nullptr;
 void* provider=CallGameFunction<void*,0x599160>();
 if(provider) {
  void* refs[6];
  // 4466D0 is stdcall. CallGameMethod supplies the ignored ECX and correct callee stack cleanup.
  CallGameMethod<void,0x4466d0>(static_cast<void*>(nullptr),refs,sizeof(void*),6,GameAddress<0x60ae20>());
  for(unsigned i=0;i<6;++i) {
   if(refs[i]) {void* old=refs[i];refs[i]=nullptr;CallGameMethod<void,0x59a990>(old);}
   if(!CallGameVirtual<uint8_t,4>(provider,&refs[i])) break;
  }
  auto cleanup=[&]() {for(int i=5;i>=0;--i) if(refs[i]) CallGameMethod<void,0x59a990>(refs[i]);};
  if(!refs[5]) {fail();cleanup();return false;}
  p792=refs[2];p796=refs[3];p788=refs[4];p780=refs[0];p784=refs[1];p800=refs[5];
  for(auto& ref:refs) ref=nullptr;
  auto connect=[&](void* ref,void* callback) {
   auto* item=static_cast<SceneCallback*>(ref);item->callback=callback;item->context=this;
   CallGameMethod<void,0x59a820>(ref,10);
  };
  connect(p784,GameAddress<0x6eeb30>());connect(p792,GameAddress<0x6f3c30>());
  connect(p796,GameAddress<0x6f3c50>());connect(p788,GameAddress<0x6eda60>());
  connect(p800,GameAddress<0x6eeb50>());
  cleanup();
 }
 void* events=CallGameFunction<void*,0x44c470>();
 CallGameVirtual<void,9>(events,this,0xfc6aa1feu,0,0);
 uint32_t key[4]{};
 if(!CallGameFunction<uint8_t,0x4aece0>(key,L"dropShadow_object_atlas",GameAddress<0xb2d882>(),0,0,0)) return false;
 allocation=AllocateScene(24,"Renderer/DropShadowProxy");
 p760=allocation?ConstructProxy723020(static_cast<ResourceProxy*>(allocation),key,false):nullptr;
 const bool loaded=LoadProxy722F50(p760,key,true,0,0); // EA calls on null when allocation fails.
 auto update=[&](void* resource) {
  if(u756<table->count) {
   auto& entry=table->entries[u756];++entry.version;entry.generation=table->generation;entry.resource=resource;
  }
 };
 if(!loaded) {update(nullptr);return false;}
 update(CallGameMethod<void*,0x722f30>(p760));
 u1352=param("LastFurDepthTexture",1,5);u1356=param("ClipMultiplier",1,2);u1360=param("ReflectionPass",1,1);
 auto* config=CallGameFunction<SceneConfig*,0x58e3f0>();
 if(config) SceneGlobal<int>(0x115b17c)=(std::atol(config->text)>=4)+1;
 return true;
}




}
