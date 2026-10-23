#pragma once
#include <cstdint>
#include "ts3/layout.h"
namespace ts3::deviceconfig {
// Foreign structures keep embedded pointers pointer-sized in the native build.
struct DeviceState {
    void* vtable; void* optionsVtable; uint8_t prefix[41];
    uint8_t recognized; uint8_t pad32[70];
    uint8_t mac; uint8_t pad79[19];
    uint32_t resolution[8]; uint8_t resolutionFlag; uint8_t padAD[15];
    int32_t error; uint8_t padC0[56];
    void* options; uint8_t padFC[28];
    const char* processor; uint8_t pad11C[28];
    const char* configuration; uint8_t pad13C[28];
    int32_t currentResolution; uint8_t pad15C[4]; uint8_t errorLine;
    uint8_t pad161[327]; uint8_t aa; uint8_t pad2A9[67]; uint32_t process;
};
TS3_X86_OFFSET(DeviceState,optionsVtable,4);
TS3_X86_OFFSET(DeviceState,resolution,140);
TS3_X86_OFFSET(DeviceState,error,188);
TS3_X86_OFFSET(DeviceState,options,248);
TS3_X86_OFFSET(DeviceState,processor,280);
TS3_X86_OFFSET(DeviceState,configuration,312);
TS3_X86_OFFSET(DeviceState,currentResolution,344);
TS3_X86_OFFSET(DeviceState,errorLine,352);
TS3_X86_OFFSET(DeviceState,aa,680);
TS3_X86_OFFSET(DeviceState,process,748);
TS3_X86_SIZE(DeviceState,752);
// TS3W.exe 0x0058ABA0: Read, convert and normalize an integer configuration setting.
int32_t ReadInteger(const void* section, const char16_t* key, int32_t fallback);
// TS3W.exe 0x0058E3F0: Return the device singleton.
DeviceState* GetDevice();
// TS3W.exe 0x0058E1F0: Notify the device's event interface.
uintptr_t NotifyDevice(void* self);
// TS3W.exe 0x00692F90: Dispatch a memory-limit command and destroy its payload.
uint8_t DispatchMemory(uint32_t first,uint32_t second);
struct ShaderSettings {
    uint8_t prefix[388]; uint32_t small, large;
    // TS3W.exe 0x006E1F90: Register render-target sizes.
    int32_t RegisterSizes(void* target);
    // TS3W.exe 0x006E31E0: Refresh FSAA and renderer settings.
    int32_t Refresh();
};
TS3_X86_OFFSET(ShaderSettings,small,388);
TS3_X86_SIZE(ShaderSettings,396);
struct ShaderListener {
    // TS3W.exe 0x006E34B0: React to the settings-changed notification.
    uint8_t OnEvent(int32_t event, const void* payload);
};
struct ResourceSettings {
    uint8_t prefix[48]; uint32_t first,second,limit; uint8_t pad3C[52];
    int32_t pending[4]; uint8_t pad80[776]; void* service;
    // TS3W.exe 0x007274D0: Reset pending operations and create the service.
    uint8_t Initialize();
};
TS3_X86_OFFSET(ResourceSettings,pending,112);
TS3_X86_OFFSET(ResourceSettings,service,904);
TS3_X86_SIZE(ResourceSettings,908);
// TS3W.exe 0x00740D80: Copy the first three resolution words and flag.
uint8_t GetResolutionList(uint32_t* a,uint32_t* b,uint32_t* c,uint8_t* flag);
// TS3W.exe 0x00740DC0: Read current resolution, or zero.
int32_t GetCurrentResolution();
// TS3W.exe 0x00740DE0: Read error code, or minus one.
int32_t GetErrorCode();
// TS3W.exe 0x00740E00: Read error-line byte, or zero.
uint8_t GetErrorLine();
// TS3W.exe 0x00740E20: Read recognition flag after checking process ownership.
uint8_t IsCardRecognized();
// TS3W.exe 0x00740E60: Test ReportOldGraphicsDriver (EA's surprising mapping).
int32_t IsCardSupported();
// TS3W.exe 0x00740E80: Read AA flag.
uint8_t IsAASupported();
// TS3W.exe 0x00740EA0: Negate DisallowEdgeSmoothing (EA's surprising mapping).
int32_t IsDriverOld();
// TS3W.exe 0x00740EC0: Read Mac flag.
uint8_t IsMac();
// TS3W.exe 0x00740ED0: Resolve option name and read its integer value.
int32_t SetOption(uint32_t key,int32_t fallback);
// TS3W.exe 0x00740F70: Refresh device properties and send notification.
uintptr_t SetProperty();
// TS3W.exe 0x00740F90: Apply device options through its embedded interface.
uintptr_t ApplyOptions();
// TS3W.exe 0x00741130: Parse new-configuration text.
int32_t IsNewConfiguration();
// TS3W.exe 0x00741150: Parse processor-level text.
int32_t GetProcessorLevel();
// TS3W.exe 0x00741230: Convert name and fetch video-card level.
uint8_t GetVideoCardLevel(uint32_t name,void* out);
// TS3W.exe 0x007412B0: Convert name and fetch option.
uint8_t GetOption(uint32_t name,void* out);
// TS3W.exe 0x00741330: Convert name and fetch default option.
uint8_t GetOptionDefault(uint32_t name,void* out);
struct ResolutionSettings {
    uint8_t prefix[36]; uint32_t resolution[8]; uint8_t flag;
    // TS3W.exe 0x007F15C0: Copy resolution information, leaving state unchanged on null.
    uint8_t CopyResolution();
};
TS3_X86_OFFSET(ResolutionSettings,resolution,36);
TS3_X86_SIZE(ResolutionSettings,72);
struct SwarmNode { void* unused; void* value; SwarmNode* next; };
TS3_X86_SIZE(SwarmNode,12);
struct SwarmSettings {
    uint8_t prefix[856]; uint8_t quality; uint8_t pad359[15];
    SwarmNode** buckets; uint32_t bucketCount;
    // TS3W.exe 0x009164F0: Reconfigure all 96 swarm groups when quality changes.
    int32_t RefreshQuality();
};
TS3_X86_OFFSET(SwarmSettings,buckets,872);
TS3_X86_SIZE(SwarmSettings,880);
struct SwarmGroup {
    uint8_t prefix[48]; void** items; uint8_t pad34[28]; uint32_t* begin; uint32_t* end;
    // TS3W.exe 0x0091BA60: Update items matching signed group byte.
    int32_t Update(int32_t group);
};
TS3_X86_OFFSET(SwarmGroup,begin,80);
TS3_X86_SIZE(SwarmGroup,88);
// TS3W.exe 0x009638B0: Set map-view memory limits (stdcall).
#if defined(_M_IX86)
uint8_t __stdcall SetMemoryLimits(uint8_t enabled);
#else
uint8_t SetMemoryLimits(uint8_t enabled);
#endif
struct LotSettings {
    uint8_t prefix[96]; void* service; uint8_t pad64[128]; uint32_t maxLots;
    uint32_t unused; float cameraThreshold;
    // TS3W.exe 0x00C60C00: Read lot-count clamp and camera-speed threshold.
    void RefreshLots();
};
TS3_X86_OFFSET(LotSettings,maxLots,228);
TS3_X86_OFFSET(LotSettings,cameraThreshold,236);
TS3_X86_SIZE(LotSettings,240);
}
