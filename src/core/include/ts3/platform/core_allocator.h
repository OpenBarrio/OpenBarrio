#pragma once
#include "ts3/allocator/core_allocator.h"

namespace ts3::platform {
// Standalone allocator for native builds; never substitutes the x86 game's allocator.
#if !defined(_M_IX86)
allocator::ICoreAllocator* CoreAllocator();
#endif
}
