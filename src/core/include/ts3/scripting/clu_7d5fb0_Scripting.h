#pragma once

namespace ts3::scripting {

// TS3W.exe 0x007D5FB0: allocates and initializes an 0x58-byte scripting prototype resource factory.
void* __fastcall CreatePrototypeResourceFactory(void* self, void* edx);

}  // namespace ts3::scripting
