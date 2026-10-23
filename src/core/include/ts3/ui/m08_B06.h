#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::ui::b06 {
struct Link { Link* next; Link* previous; };
struct Point { float x,y; };
// Sparse views: padding denotes unobserved bytes, not inferred ownership.
struct Window {
    void* primary; void* secondary; Link child; uint8_t opaque10[8];
    Link active, pending; uint8_t opaque28[4]; Window* parent;
    uint8_t opaque30[0x40]; uint32_t flags; void* cursor;
    uint8_t opaque78[0xD8]; uint8_t transform[0x44];
    uint8_t opaque194[0x64]; uint8_t state; uint8_t opaque1F9[0x17]; Link children;
};
struct Manager {
    void* primary; Window* main; Link list8,active,pending; Link* current;
    uint8_t changed; uint8_t opaque25[0xB]; uint8_t draining;
    uint8_t opaque31[0x97]; void* cursorService; Point position; void* cursor; void* source;
    uint8_t opaqueDC[0x600]; Window* focus;
};
TS3_X86_OFFSET(Window,parent,0x2C);
TS3_X86_OFFSET(Window,flags,0x70);
TS3_X86_OFFSET(Window,transform,0x150);
TS3_X86_OFFSET(Window,state,0x1F8);
TS3_X86_OFFSET(Window,children,0x210);
TS3_X86_OFFSET(Manager,cursorService,0xC8);
TS3_X86_OFFSET(Manager,source,0xD8);
TS3_X86_OFFSET(Manager,focus,0x6DC);
// TS3W.exe 0x0050AB90: Exchange two intrusive list heads, repairing sentinels.
Link* __fastcall SwapHeads(Link*,void*,Link*);
// TS3W.exe 0x0050AC30: Return the main window's adjusted secondary interface, or null.
void* __fastcall MainInterface(Manager*,void*);
// TS3W.exe 0x0050AC40: Forward a position message through manager slot 6.
uintptr_t __fastcall PositionMessage(Manager*,void*,float,float);
// TS3W.exe 0x0050AC90: Retain and forward an object message through manager slot 6.
uintptr_t __fastcall ObjectMessage(Manager*,void*,uint32_t,void*);
// TS3W.exe 0x0050ACE0: Replace the retained source, retaining before releasing.
void __fastcall SetSource(Manager*,void*,void*);
// TS3W.exe 0x0050AD20: Select the first ancestor cursor and notify on change.
void __fastcall RefreshCursor(Manager*,void*);
// TS3W.exe 0x0050AD60: Replace the cursor service and replay a non-null cursor even on identity.
void __fastcall SetCursorService(Manager*,void*,void*);
// TS3W.exe 0x0050ADC0: Copy the stored position and return the destination.
Point* __fastcall GetPosition(Manager*,void*,Point*);
// TS3W.exe 0x0050ADE0: Convert floats with EA's magic-bias bit arithmetic and forward.
void __fastcall SetPosition(Manager*,void*,const Point*);
// TS3W.exe 0x0050AE40: Update active membership and repair an in-progress iterator.
void __fastcall UpdateActive(Manager*,void*,Window*);
// TS3W.exe 0x0050AEC0: Refresh cursor only when the supplied window is a focus ancestor.
void __fastcall AncestorChanged(Manager*,void*,Window*);
// TS3W.exe 0x0050AEF0: Queue a window once unless draining has begun.
Link* __fastcall QueuePending(Manager*,void*,Window*);
// TS3W.exe 0x0050AF20: Mark draining and unlink pending windows from the tail.
void __fastcall DrainPending(Manager*,void*);
// TS3W.exe 0x0050AF60: Recursively hit-test children before the candidate, preserving AL checks.
Window* __fastcall HitTest(Manager*,void*,Window*,const Point*,uint32_t,Point*);
// TS3W.exe 0x0050B0B0: Copy the aligned transform block and return its final word.
uint32_t __fastcall CopyTransform(void*,void*,void*,const void*);
}
