#pragma once
#include "ts3/layout.h"
#include <cstdint>

namespace ts3::ui44d890 {
// TS3W.exe 0x0044DA50: CRT decimal conversion, returned in ST0.
double ParseDouble(const char* text);
// TS3W.exe 0x00453040: unsigned decimal conversion.
uint32_t ParseUnsigned(const char* text);
// TS3W.exe 0x0045F0F0: FNV-1, with zero for null/empty strings.
uint32_t EffectNameHash(const char* text);
// TS3W.exe 0x00470A60: ASCII folding; differences use signed bytes.
int CompareAscii(const char* left, const char* right);
// TS3W.exe 0x00470D60: reads one token, consuming trailing delimiters.
bool ReadToken(const char* input, uint32_t length, char delimiter, char* output,
               uint32_t capacity, const char** next);
struct Buffer {
    void* begin;
    uint32_t size, capacity, field0c, field10;
    void* allocator;
    // TS3W.exe 0x00467D30: frees storage and clears first five words.
    void Reset();
};
TS3_X86_OFFSET(Buffer, allocator, 0x14);
TS3_X86_OFFSET(Buffer, begin, 0);
TS3_X86_OFFSET(Buffer, size, 4);
TS3_X86_OFFSET(Buffer, capacity, 8);
TS3_X86_OFFSET(Buffer, field0c, 0x0c);
TS3_X86_OFFSET(Buffer, field10, 0x10);
TS3_X86_SIZE(Buffer, 0x18);
// TS3W.exe 0x0044D890: optionally constructs and retains the global font server.
void* GetFontServer(bool create);
// TS3W.exe 0x009EF0C0: creates the font server and returns true.
bool InitializeFontServer();
// TS3W.exe 0x009EF1B0: creates the font server and returns it.
void* EnsureFontServer();
// TS3W.exe 0x00453160: capacity is ignored, including on malformed commands.
int ParseEffect(const char* text, uint32_t* output, int capacity, int* errorLine);
// TS3W.exe 0x00454560: initializes effect settings and parses the command words.
int InitializeEffect(const char* text, uint32_t* settings, int* errorLine);
// TS3W.exe 0x00691D20: parses and registers an effect from UTF-16 text.
void RegisterEffect(uint32_t id, const char16_t* text);
// TS3W.exe 0x00691DE0: handles font/effectID/effectName console commands.
void FontCommand(int count, const char16_t* const* arguments);
}
