#pragma once

namespace ts3::eatextutil {

// TS3W.exe 0x004FDCE0.
char* CopyString004FDCE0(char* destination, const char* source);

// TS3W.exe 0x004FF500.
char* LowerAscii004FF500(char* text);

// TS3W.exe 0x00500080.
bool MatchWildcard00500080(const char* text, const char* pattern);

// Implements the non-fast-path body entered at 0x0050023F from 0x00500210.
// The original address is an interior jump target, not a callable ABI entry.
bool NormalizeAndMatch0050023F(const char* text, const char* pattern);

}  // namespace ts3::eatextutil
