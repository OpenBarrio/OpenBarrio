// TS3W.exe 0x00722F00, 24 bytes: leaf thiscall predicate.
#include "ts3/ui/clu_722f00_UI.h"

namespace ts3::ui {

bool UiFlagPredicate::IsAlignedHandle() const
{
    // The original tests the pointer value, then only its low two bits.
    return handle != 0 && (handle & 3u) == 0;
}

} // namespace ts3::ui
