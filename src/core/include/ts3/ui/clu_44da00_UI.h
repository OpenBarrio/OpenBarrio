#pragma once
#include "ts3/layout.h"
#include <cstdint>

namespace ts3::ui {
// All fields in this descriptor are scalar IDs/flags (no embedded pointers).
struct UiTextDescriptor {
    std::uint32_t fields[9];
    std::uint8_t mode, flag1, flag2, padding;
    std::uint32_t tail[2];
    // TS3W.exe 0x005236E0: reset the descriptor, preserving byte 39; return this.
    UiTextDescriptor* Reset();
};
TS3_X86_OFFSET(UiTextDescriptor, mode, 36);
TS3_X86_OFFSET(UiTextDescriptor, tail, 40);
TS3_X86_SIZE(UiTextDescriptor, 48);

struct UiWordVector {
    std::uint16_t *begin, *end, *capacity;
    // TS3W.exe 0x0045E120: resize with zero fill; shrinking does a zero-byte memmove.
    void* Resize(std::uint32_t count);
};
TS3_X86_OFFSET(UiWordVector, end, 4);
TS3_X86_SIZE(UiWordVector, 12);

struct UiGlyphRun {
    void* style;
    void* font;
    std::uint32_t flags;
};
TS3_X86_OFFSET(UiGlyphRun, flags, 8);
TS3_X86_SIZE(UiGlyphRun, 12);
struct UiFontNode {
    UiFontNode *parent, *left, *right;
    std::uint32_t color;
    void* font;
};
struct UiFontAnchor {
    UiFontNode *parent, *first, *last;
    std::uint32_t color;
};
TS3_X86_OFFSET(UiFontNode, font, 16);
TS3_X86_SIZE(UiFontNode, 20);
TS3_X86_SIZE(UiFontAnchor, 16);
// Recovered font-selection fields and run storage use native pointers.
// Unused engine fields remain opaque; the checks retain the game's x86 offsets.
struct UiLayoutSegment { std::int32_t style, variant; std::uint32_t start, end; };
struct UiLayoutVector { void *begin, *end, *capacity; std::uint32_t opaque[3]; };
TS3_X86_SIZE(UiLayoutSegment, 16);
TS3_X86_SIZE(UiLayoutVector, 24);
struct UiGlyphContext {
    std::uint8_t prefix[28];
    UiFontAnchor fontAnchor;
    std::uint32_t fontCount;
    std::uint8_t opaque48[200];
    std::uint8_t useFontServer;
    std::uint8_t opaque249[595];
    std::int32_t clipMode;
    std::uint8_t opaque848[16];
    std::uint32_t fontMode;
    std::uint8_t opaque868[28];
    void* fontServer;
    void** defaultStyle;
    std::uint8_t opaque904[52];
    float widthLimit;
    std::uint8_t opaque960[8];
    std::uint8_t overflowAllowed, opaque969;
    std::uint16_t fallbackCharacter;
    std::uint8_t opaque972[28];
    std::uint32_t prefixCount, suffixCount;
    std::uint32_t currentPosition[3], savedPosition[3], targetPosition[3];
    std::int32_t layoutState;
    std::uint32_t wordTag;
    UiWordVector words;
    std::uint8_t opaque1064[12];
    UiGlyphRun* runs;
    std::uint8_t opaque1080[20];
    UiWordVector glyphWords;
    std::uint8_t opaque1112[12];
    UiLayoutVector glyphData, metrics, wordToGlyph, otherData;
    std::uint32_t visibleCount, opaque1224;
    float visibleWidth;
    std::uint8_t opaque1232[12];
    std::uint32_t finalized, opaque1248[2], processedCount;
    void *segmentsBegin, *segmentsEnd, *segmentsCapacity;
    std::uint8_t opaque1272[72];
    std::uint8_t segmented, opaque1345[3];
    std::uint32_t rebuildState1, rebuildState2;
    // TS3W.exe 0x00463320: choose fonts for a word range, retaining the normal font after a fallback.
    void SelectFonts(std::uint32_t start, std::uint32_t end);
    // TS3W.exe 0x00465F40: reset and rebuild only while layoutState differs from one.
    std::int32_t Rebuild();
    // TS3W.exe 0x004635A0: select the visible word boundary under the width limit.
    void Clip();
    // TS3W.exe 0x00464DB0: choose fonts, build style segments, and emit runs; EAX is incidental.
    void BuildLayout();
    // TS3W.exe 0x00466990: truncate visible layout, advance positions, and finalize.
    int Finalize();
    // TS3W.exe 0x0046C3C0: shape and emit a word chunk using a fixed run; return glyph count.
    int EmitChunk(std::uint32_t start, int span, const std::uint16_t* words, std::uint32_t count);
    // TS3W.exe 0x0046C480: obtain and emit successive chunks of [start,end).
    void EmitRange(std::uint32_t start, std::uint32_t end);
};
TS3_X86_OFFSET(UiGlyphContext, clipMode, 844);
TS3_X86_OFFSET(UiGlyphContext, widthLimit, 956);
TS3_X86_OFFSET(UiGlyphContext, currentPosition, 1008);
TS3_X86_OFFSET(UiGlyphContext, glyphWords, 1100);
TS3_X86_OFFSET(UiGlyphContext, metrics, 1148);
TS3_X86_OFFSET(UiGlyphContext, visibleCount, 1220);
TS3_X86_OFFSET(UiGlyphContext, processedCount, 1256);
TS3_X86_OFFSET(UiGlyphContext, runs, 1076);
TS3_X86_OFFSET(UiGlyphContext, fontAnchor, 28);
TS3_X86_OFFSET(UiGlyphContext, fontCount, 44);
TS3_X86_OFFSET(UiGlyphContext, useFontServer, 248);
TS3_X86_OFFSET(UiGlyphContext, fontMode, 864);
TS3_X86_OFFSET(UiGlyphContext, fontServer, 896);
TS3_X86_OFFSET(UiGlyphContext, defaultStyle, 900);
TS3_X86_OFFSET(UiGlyphContext, fallbackCharacter, 970);
TS3_X86_OFFSET(UiGlyphContext, layoutState, 1044);
TS3_X86_OFFSET(UiGlyphContext, wordTag, 1048);
TS3_X86_OFFSET(UiGlyphContext, words, 1052);
TS3_X86_OFFSET(UiGlyphContext, segmentsBegin, 1260);
TS3_X86_OFFSET(UiGlyphContext, segmentsEnd, 1264);
TS3_X86_OFFSET(UiGlyphContext, rebuildState1, 1348);
TS3_X86_OFFSET(UiGlyphContext, rebuildState2, 1352);
TS3_X86_SIZE(UiGlyphContext, 1356);
// Pointer-bearing tree headers grow naturally on x64. The bridged lookup owns
// ordering and traversal; this unit only tests its end iterator and returns data.
struct UiPropertyNodeHeader {
    void* links[3];
    std::uint32_t color;
    std::int32_t key;
    std::uint32_t reserved;
};
struct UiPropertyTree {
    std::uint32_t allocator;
    UiPropertyNodeHeader sentinel;
    std::uint32_t count;
    void* storage;
    // TS3W.exe 0x00526EE0: look up outer/inner keys; leave output unchanged on miss.
    bool Find(std::int32_t innerKey, std::int32_t outerKey, void* output);
};
TS3_X86_SIZE(UiPropertyNodeHeader, 24);
TS3_X86_OFFSET(UiPropertyTree, sentinel, 4);
TS3_X86_SIZE(UiPropertyTree, 36);

struct UiDescriptorNode { void* next; void* descriptor; };
struct UiDescriptorLock {
    void* debug;
    std::int32_t lockCount, recursionCount;
    void* owner;
    void* semaphore;
    std::uintptr_t spin;
};
struct UiDescriptorRegistry {
    std::uint8_t prefix[12];
    std::uint32_t mapTag;
    UiDescriptorNode** buckets;
    std::uint32_t sentinelIndex;
    std::uint8_t reserved[232];
    UiDescriptorLock lock;
    // TS3W.exe 0x0045F200: locked byte-key lookup; optionally copy exactly 632 bytes.
    void* Lookup(std::uint32_t key, void* destination);  // key: the whole dword (0xD38B00 hashes 4 bytes)
};
TS3_X86_OFFSET(UiDescriptorRegistry, mapTag, 12);
TS3_X86_OFFSET(UiDescriptorRegistry, buckets, 16);
TS3_X86_OFFSET(UiDescriptorRegistry, lock, 256);
TS3_X86_SIZE(UiDescriptorLock, 24);
TS3_X86_SIZE(UiDescriptorRegistry, 280);
struct UiWidthProperty {
    std::uint32_t kind, opaque[2];
    void* value;
};
struct UiWidthContext {
    std::uint8_t opaque0[592];
    void* descriptorVtable;
    std::uint8_t opaque596[44];
    float spacing1;
    std::uint32_t opaque644;
    float spacing2;
    std::uint8_t opaque652[60];
    float defaultWidth;
    std::uint8_t opaque716[180];
    UiPropertyTree properties;
    // TS3W.exe 0x00528660: measure a property or text layout, restoring descriptor mode.
    double Measure(std::int32_t innerKey, std::int32_t outerKey, void* property);
};
TS3_X86_OFFSET(UiWidthProperty, value, 12);
TS3_X86_OFFSET(UiWidthContext, descriptorVtable, 592);
TS3_X86_OFFSET(UiWidthContext, spacing1, 640);
TS3_X86_OFFSET(UiWidthContext, spacing2, 648);
TS3_X86_OFFSET(UiWidthContext, defaultWidth, 712);
TS3_X86_OFFSET(UiWidthContext, properties, 896);
// TS3W.exe 0x0044DA00: return the shared descriptor registry, lazily constructing it.
UiDescriptorRegistry* GetDescriptorRegistry(std::uint8_t create);
} // namespace ts3::ui
