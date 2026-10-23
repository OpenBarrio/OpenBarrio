#include "ts3/anim/m05_M64.h"
#include "ts3/layout.h"
namespace ts3::anim {
namespace {
void* Destroy(void* self, uint32_t flags, unsigned kind, uint32_t key)
{
#if defined(_M_IX86)
    static const uintptr_t tables[] = {0xfc24d0,0xfc24e0,0xfc2500,0xfc2508,0xfc2510,0xfc2518};
    *static_cast<void**>(self) = reinterpret_cast<void*>(tables[kind]);
    if (kind == 0)
        CallGameFunction<void, 0x0054C510>(key, self);
    else
        CallGameFunction<void, 0x00565D30>(key, self);
#else
    (void)kind; (void)key;
    AbortNotRewritten("m05_M64 original vtable and registry");
#endif
    if (flags & 1)
        CallGameFunction<void, 0x004E3F70>(self);
    return self; // EA returns the saved pointer even after deletion.
}
}
void* m64::Destroy00571540(uint32_t flags)
{
    return Destroy(this, flags, 0, 0x0BB6D0A5);
}
void* m64::Destroy00571570(uint32_t flags)
{
    return Destroy(this, flags, 1, 0xB489FC2C);
}
void* m64::Destroy005715A0(uint32_t flags)
{
    return Destroy(this, flags, 2, 0x073B9D8C);
}
void* m64::Destroy005715D0(uint32_t flags)
{
    return Destroy(this, flags, 3, 0x34F49F94);
}
void* m64::Destroy00571600(uint32_t flags)
{
    return Destroy(this, flags, 4, 0x06C12D61);
}
void* m64::Destroy00571630(uint32_t flags)
{
    return Destroy(this, flags, 5, 0x06B26363);
}
}
