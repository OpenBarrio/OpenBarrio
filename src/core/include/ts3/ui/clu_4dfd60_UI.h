// Cluster unit clu_4dfd60_UI (subsystem UI): one TS3W.exe leaf (0x004DFD60) that locates
// the start of the last path component inside a word range [begin, end). Called from
// 0x004575C0 (buffer + end-of-buffer global; the caller takes (result - begin) >> 1 as the
// last component's length in words) and from the 0x004DFFD0 wrapper (a {begin, end} pair).
//
//   0x004DFD60  FindLastPathComponent  cdecl, ret 0: WORD* (WORD* begin, WORD* end)
//
// The string is a word array (each character a 16-bit word, char in the low byte, 0 in
// the high byte, NUL word terminated). `end` == the game global 0x011C7E2C (all 0xFF
// in the real game image, 0 in the raw file) means "the NUL terminator of the string at
// begin"; the rewrite reads the global from the mapped image so it matches the original
// in every environment. Separators are '\\', '/' and ':'.
#pragma once

#include <cstdint>

namespace ts3::ui {

// TS3W.exe 0x004DFD60: start of the last path component in [begin, end) - the position
// just after the last separator in the range, or begin when there is none. EA quirks
// kept (see the .cpp): a range whose last word is a separator returns end, and a string
// beginning with "\\" whose first character is the only separator found also returns end.
uint16_t* FindLastPathComponent(uint16_t* begin, uint16_t* end);

}  // namespace ts3::ui
