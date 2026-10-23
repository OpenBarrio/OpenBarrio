#pragma once
#include "ts3/layout.h"
namespace ts3::resource::m03::g24 {
// TS3W.exe 0x00736DB0: construct the main ResourceSystem. Installs the abstract/primary/manager/observer
// tables (manager embedded at +8), constructs the three shadow-queue mutexes at +0x218/+0x248/+0x278, the
// flush stopwatch at +0x2C8 and the preloader at +0x768, then registers the system with the tuning object.
// Returns the object. EA quirks kept: the third tuning call runs even when it is the first one to see a
// null object and passes 0/0x7FFFFFFF; the manager constructor always receives a null allocator.
void* __fastcall Construct(void*,void*);
// TS3W.exe 0x00737060: stop the ResourceSystem. Releases the two shadow databases at +0x1E0/+0x1E4 (manager
// slot 21, the cache-menu unregister helper, slot 2 and the last-ref reset-to-one before slot 0), releases
// the async singleton, then sets the flush-queue enabled byte and drains with an infinite timeout. Returns
// the full EAX of the last flush call; only its low byte decides the loop. EA quirk: the second database
// skips the cache-menu helper; the first release's result is discarded.
int __fastcall Stop(void*,void*);
// TS3W.exe 0x00737560: tick with a millisecond budget (ret 4). Clamps to 250 while +0x1F0 is set, drains
// the flush queue once, then under the +0x248 mutex reclaims the +0x204 vector when the +0x2C8 stopwatch
// exceeds 1000 units (stop, take begin/end, clear all three fields). Under the +0x218 mutex every +0x2B8
// watcher gets slot 6(0); the async singleton receives a deadline built in a 32-byte local stopwatch; the
// preloader is ticked and the system is flushed. Returns the preloader result, or the free result when a
// drained vector was reclaimed.
int __fastcall Update(void*,void*,int budget);
}