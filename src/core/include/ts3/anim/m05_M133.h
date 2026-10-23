#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m133_impl {
struct Body {
    void** vtable;
    void** releaseTable;
    uint32_t refs;
    unsigned char pad0c[8];
    void* host;
    void** notification;
    unsigned char pad1c[0x60];
    uint32_t keys[4];
    unsigned char pad8c;
    uint8_t disabled;
    unsigned char pad8e[6];
    void* stylesBegin;
    void* stylesEnd;
    unsigned char pad9c[0x14];
    unsigned char* slaves;
    void** slaveHosts;
    unsigned char padb8[0xc];
    void* master;
    int32_t slaveCount;
    unsigned char padcc[0x58];
    void* actor;
    unsigned char pad128[0x1c];
    float speeds[6];
    unsigned char pad15c[0x1c];
    uint32_t active;
    unsigned char pad17c[8];
    uint8_t routeEnabled;
    unsigned char pad185[3];
    unsigned char* progression;
    void* progressionEnd;
    void* progressionCapacity;
    unsigned char pad194[0xb4];
    uint32_t index;
};
TS3_X86_OFFSET(Body, notification, 0x18);
TS3_X86_OFFSET(Body, keys, 0x7c);
TS3_X86_OFFSET(Body, slaves, 0xb0);
TS3_X86_OFFSET(Body, master, 0xc4);
TS3_X86_OFFSET(Body, actor, 0x124);
TS3_X86_OFFSET(Body, speeds, 0x144);
TS3_X86_OFFSET(Body, active, 0x178);
TS3_X86_OFFSET(Body, progression, 0x188);
TS3_X86_OFFSET(Body, index, 0x248);
// TS3W.exe 0x0084CF00: dispatch controller-start style/environment updates; preserves ref restoration and secondary-this deletion.
uintptr_t OnControllerStarted(void* notification, void** controller, uint32_t event);
}
namespace ts3::anim {
struct m133 {
    uint32_t OnControllerStarted(void* controller,uint32_t event) {
        return static_cast<uint32_t>(m133_impl::OnControllerStarted(this,static_cast<void**>(controller),event));
    }
};
}
