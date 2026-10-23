#include "ts3/anim/m05_M62.h"
namespace ts3::anim::m62 {
// The registry and allocator remain original game boundaries.
static void* DestroyScratch(void* self, uint32_t flags, void* table, uint32_t key) {
    *static_cast<void**>(self) = table;
    CallGameFunction<void, 0x0054C510>(key, self);
    if (flags & 1) {
        CallGameFunction<void, 0x004E3F70>(self);
    }
    return self;
}
void* Destroy1300(void* self, uint32_t flags) {
#if defined(_M_IX86)
    return DestroyScratch(self, flags, reinterpret_cast<void*>(0x00FC23B0), 0xF3F23A79u);
#else
    (void)self; (void)flags;
    NotRewritten("Scratch deleting destructor needs game registry");
#endif
}
void* Destroy1330(void* self, uint32_t flags) {
#if defined(_M_IX86)
    return DestroyScratch(self, flags, reinterpret_cast<void*>(0x00FC23C0), 0x93DFFDC3u);
#else
    (void)self; (void)flags;
    NotRewritten("Scratch deleting destructor needs game registry");
#endif
}
void* Destroy1360(void* self, uint32_t flags) {
#if defined(_M_IX86)
    return DestroyScratch(self, flags, reinterpret_cast<void*>(0x00FC23D0), 0x5419E34Eu);
#else
    (void)self; (void)flags;
    NotRewritten("Scratch deleting destructor needs game registry");
#endif
}
void* Destroy1390(void* self, uint32_t flags) {
#if defined(_M_IX86)
    return DestroyScratch(self, flags, reinterpret_cast<void*>(0x00FC2410), 0x7419E37Cu);
#else
    (void)self; (void)flags;
    NotRewritten("Scratch deleting destructor needs game registry");
#endif
}
void* Destroy13C0(void* self, uint32_t flags) {
#if defined(_M_IX86)
    return DestroyScratch(self, flags, reinterpret_cast<void*>(0x00FC2450), 0xB3F291F8u);
#else
    (void)self; (void)flags;
    NotRewritten("Scratch deleting destructor needs game registry");
#endif
}
void* Destroy13F0(void* self, uint32_t flags) {
#if defined(_M_IX86)
    return DestroyScratch(self, flags, reinterpret_cast<void*>(0x00FC2460), 0x06B264EAu);
#else
    (void)self; (void)flags;
    NotRewritten("Scratch deleting destructor needs game registry");
#endif
}
#if defined(_M_IX86)
void* __fastcall hooks::Destroy1300(void* self, void*, uint32_t flags) { return m62::Destroy1300(self, flags); }
#endif
#if defined(_M_IX86)
void* __fastcall hooks::Destroy1330(void* self, void*, uint32_t flags) { return m62::Destroy1330(self, flags); }
#endif
#if defined(_M_IX86)
void* __fastcall hooks::Destroy1360(void* self, void*, uint32_t flags) { return m62::Destroy1360(self, flags); }
#endif
#if defined(_M_IX86)
void* __fastcall hooks::Destroy1390(void* self, void*, uint32_t flags) { return m62::Destroy1390(self, flags); }
#endif
#if defined(_M_IX86)
void* __fastcall hooks::Destroy13C0(void* self, void*, uint32_t flags) { return m62::Destroy13C0(self, flags); }
#endif
#if defined(_M_IX86)
void* __fastcall hooks::Destroy13F0(void* self, void*, uint32_t flags) { return m62::Destroy13F0(self, flags); }
#endif
}
