#pragma once

namespace ts3::scripting {
// TS3W.exe 0x005826B0: look up the globally selected binding in a non-null owner, otherwise return null.
void* FindSelectedBinding(const void* owner);
} // namespace ts3::scripting
