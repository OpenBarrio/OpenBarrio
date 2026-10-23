#pragma once

namespace ts3::anim {
// TS3W.exe 0x00570A10: Construct evaluator via 0x0054DDB0 in non-null storage, otherwise return null.
void* __stdcall Scratch570A10(void* context, void* storage);
// TS3W.exe 0x00570A40: Construct evaluator via 0x0055DED0 in non-null storage, otherwise return null.
void* __stdcall Scratch570A40(void* context, void* storage);
// TS3W.exe 0x00570A60: Construct evaluator via 0x0054EE20 in non-null storage, otherwise return null.
void* __stdcall Scratch570A60(void* context, void* storage);
// TS3W.exe 0x00570A80: Construct evaluator via 0x0054CE90 in non-null storage, otherwise return null.
void* __stdcall Scratch570A80(void* context, void* storage);
// TS3W.exe 0x00570AC0: Construct evaluator via 0x0054CF40 in non-null storage, otherwise return null.
void* __stdcall Scratch570AC0(void* context, void* storage);
// TS3W.exe 0x00570AE0: Construct evaluator via 0x0054CFA0 in non-null storage, otherwise return null.
void* __stdcall Scratch570AE0(void* context, void* storage);
}
