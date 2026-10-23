#pragma once
#include "ts3/layout.h"
// The harness already declares an incompatible legacy scratch::WideString.
// Keep its hook-facing declarations opaque; core and fixture tests opt into views.
#if defined(TS3_M08_B26_LAYOUTS)
#include "ts3/ui/m08_layouts.h"
#endif
namespace ts3::ui::b26 {
#if defined(TS3_M08_B26_LAYOUTS)
using m08::WideString;
using ts3::scratch::StringVector;
// Sparse native views; only the x86 observed extents are asserted.
struct ComboInterface {
    void* primary; void* adapter;
    int32_t selected,previous;
    void* child; void* drawable;
    uint32_t colors[10];
    uint8_t enabled,changed,pad292[2];
    StringVector items;
    uint8_t block2AC[0x30];
    float metrics[9];
    void* object300; void* object304;
    float bounds[4];
    uint8_t tail318[8];
};
struct Combo {
    m08::Window window;
    uint8_t opaque244[12];
    ComboInterface interface250;
};
struct Drawable {
    void* primary; void* secondary; uint32_t refs; void* tertiary;
    void* object10; void* object14; void* object18;
};
struct ConsoleDrawable {
    Drawable drawable;
    void* object1C; void* object20; void* object24; void* object28;
    void* owner;
};
TS3_X86_OFFSET(Combo,interface250,0x250);
TS3_X86_OFFSET(ComboInterface,items,0x44);
TS3_X86_OFFSET(ComboInterface,metrics,0x8C);
TS3_X86_OFFSET(ComboInterface,bounds,0xB8);
TS3_X86_SIZE(Combo,0x320);
TS3_X86_SIZE(Drawable,0x1C);
TS3_X86_OFFSET(ConsoleDrawable,owner,0x2C);
#else
struct Combo;
struct ComboInterface;
struct Drawable;
struct ConsoleDrawable;
#endif
// TS3W.exe 0x0051CA30: Find the first exact UTF16 item, or -1 (null query is a miss).
int32_t __fastcall Find(ComboInterface*,void*,const char16_t*);
// TS3W.exe 0x0051CB90: Return the constant interface type C1.
uint32_t __fastcall Type(void*,void*);
// TS3W.exe 0x0051CBA0: Allocate and initialize the three-interface combo drawable.
Drawable* __fastcall CreateDrawable(void*,void*,uint32_t,uint32_t);
// TS3W.exe 0x0051CC00: Initialize a combo window after its base window constructor.
Combo* __fastcall InitCombo(Combo*,void*);
// TS3W.exe 0x0051CD90: Query the combo through its +250 interface.
void* __fastcall Query250(ComboInterface*,void*,uint32_t);
// TS3W.exe 0x0051CDA0: Delete the combo through its +250 interface.
void* __fastcall Delete250(ComboInterface*,void*,uint32_t);
// TS3W.exe 0x0051CDB0: Query the combo through its +4 interface.
void* __fastcall Query4(void*,void*,uint32_t);
// TS3W.exe 0x0051CDC0: Delete the combo through its +4 interface.
void* __fastcall Delete4(void*,void*,uint32_t);
// TS3W.exe 0x0051CDD0: Delete the combo through its +254 adapter.
void* __fastcall Delete254(void*,void*,uint32_t);
// TS3W.exe 0x0051D630: Destroy vector elements, reset end and preserve callee EAX.
int32_t __fastcall Clear(ComboInterface*,void*);
// TS3W.exe 0x0051D6A0: Allocate a combo and return its adjusted +4 interface.
void* __fastcall CreateCombo(void*,void*,uint32_t,uint32_t);
// TS3W.exe 0x0051D8F0: Append a temporary UTF16 string and return cleanup EAX.
void* __fastcall Append(ComboInterface*,void*,const char16_t*);
// TS3W.exe 0x0051D980: Insert a UTF16 item at index <= size; return AL only.
uint8_t __fastcall Insert(ComboInterface*,void*,uint32_t,const char16_t*);
// TS3W.exe 0x0051DAA0: Initialize the console drawable and its borrowed owner.
ConsoleDrawable* __fastcall InitConsole(ConsoleDrawable*,void*,void*);
// TS3W.exe 0x0051DAD0: Write (20,12) only for kind 3; return AL only.
uint8_t __fastcall Measure(void*,void*,float*,uint32_t,uint32_t);
}
