#include "ts3/misc/clu_f83de0_Misc.h"

namespace ts3::misc {
// TS3W.exe 0x00F83DE0, cdecl, ret (no arguments). EAX is the atexit result.
int InitMiscRecords()
{
#if defined(_M_IX86)
    auto* records = reinterpret_cast<MiscRecord*>(0x011D3E58);
    void* const empty = reinterpret_cast<void*>(0x011C7E54);
    void* const limit = reinterpret_cast<void*>(0x011C7E55);
    const char* const name = reinterpret_cast<const char*>(0x00FFF988);
#else
    // No original image or exit-handler registry exists in the portable build.
    static MiscRecord records[10];
    static char sentinel[2];
    void* const empty = sentinel;
    void* const limit = sentinel + 1;
    const char* const name = nullptr;
#endif
    // Original EDI=4 / sub / jns executes five pairs, i.e. ten records.
    for (unsigned i = 0; i < 10; ++i) {
        records[i].begin = empty;
        records[i].end = empty;
        records[i].limit = limit;
        records[i].name = name;
    }
#if defined(_M_IX86)
    using ExitHandler = void(*)();
    return CallGameFunction<int, 0x00F7E9A1>(reinterpret_cast<ExitHandler>(0x00F925E0));
#else
    AbortNotRewritten("Misc records exit registration");
    return -1;
#endif
}
} // namespace ts3::misc
