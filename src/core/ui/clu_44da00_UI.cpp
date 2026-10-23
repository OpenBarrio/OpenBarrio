#include "ts3/ui/clu_44da00_UI.h"
#include "ts3/ui/clu_44d890_UI.h"
#include <cstring>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace ts3::ui {
namespace {
int FontStyle(std::uint32_t flags)
{
    const int low = flags & 127;
    return low < 64 ? low : low - 128;
}
}
// 0x00463320, thiscall, ret 8. No additional font references are retained.
void UiGlyphContext::SelectFonts(std::uint32_t start, std::uint32_t end)
{
    if (useFontServer) {
        if (!fontServer) fontServer = ui44d890::GetFontServer(true);
        if (fontMode == 1) {
            auto font = CallGameVirtual<void*, 8>(fontServer, *defaultStyle, 0, 0,
                                                static_cast<int>(fallbackCharacter), -1, 1);
            std::uint32_t glyph;
            if (!CallGameVirtual<int, 15>(font, &fallbackCharacter, 1, &glyph, 0, 2, 0))
                fallbackCharacter = 42;
            for (auto i = start; i < end; ++i) runs[i].font = font;
            CallGameVirtual<void, 3>(font);
        } else {
            void* currentStyle = nullptr;
            void* currentFont = nullptr;
            for (auto i = start; i < end; ++i) {
                const int character = words.begin[i];
                const int style = FontStyle(runs[i].flags);
                void* saved = nullptr;
                if (currentStyle == runs[i].style) {
                    // A null first style makes EA dereference a null font; keep that contract.
                    if (!CallGameVirtual<bool, 16>(currentFont, character, style)) {
                        saved = currentFont;
                        currentFont = CallGameVirtual<void*, 8>(fontServer, currentStyle, 0, 0, character, style, 1);
                        CallGameVirtual<void, 3>(currentFont);
                    }
                } else {
                    currentStyle = runs[i].style;
                    currentFont = CallGameVirtual<void*, 8>(fontServer, currentStyle, 0, 0, character, style, 1);
                    CallGameVirtual<void, 3>(currentFont);
                }
                runs[i].font = currentFont;
                if (saved) currentFont = saved;
            }
        }
    } else if (fontCount) {
        auto sentinel = reinterpret_cast<UiFontNode*>(&fontAnchor);
        for (auto i = start; i < end; ++i) {
            auto node = fontAnchor.first;
            const int character = words.begin[i];
            if (character >= 128 && fontCount != 1) {
                while (node != sentinel && !CallGameVirtual<bool, 16>(node->font, character, FontStyle(runs[i].flags)))
                    node = CallGameFunction<UiFontNode*, 0x004E0B20>(node);
            } else {
                node = sentinel;
            }
            if (node == sentinel) node = fontAnchor.first;
            runs[i].font = node->font;
        }
    }
}

// 0x00465F40, thiscall, ret 0. The helpers may change layoutState before it is returned.
std::int32_t UiGlyphContext::Rebuild()
{
    if (layoutState != 1) {
        rebuildState1 = 0;
        rebuildState2 = 0;
        segmentsEnd = segmentsBegin;
        CallGameMethod<void, 0x00465950>(this);
        CallGameMethod<void, 0x00464DB0>(this);
        CallGameMethod<void, 0x004635A0>(this);
    }
    return layoutState;
}
// 0x005236E0 (__thiscall, ret): byte 39 is deliberately not initialized.
UiTextDescriptor* UiTextDescriptor::Reset()
{
    for (auto& field : fields) field = 0;
    mode = 0xff;
    flag1 = 0;
    flag2 = 0;
    tail[0] = 0;
    tail[1] = 0;
    return this;
}

// 0x0045E120 (__thiscall, ret 4). The grow helper owns allocator policy.
void* UiWordVector::Resize(std::uint32_t count)
{
    const auto size = static_cast<std::uint32_t>(
        (reinterpret_cast<std::intptr_t>(end) - reinterpret_cast<std::intptr_t>(begin)) / 2);
    if (count > size) {
        std::uint32_t zero = 0;
        return CallGameMethod<void*, 0x0045DDA0>(this, end, count - size, &zero);
    }
    auto* destination = begin ? begin + count : nullptr;
    // EA calls memmove(destination, end, 0), returning destination. No words
    // are cleared on shrink, including the newly-unused suffix.
    end = destination;
    return destination;
}

// 0x0046C3C0 (__thiscall, ret 16). The original's 32-word scratch buffer and
// fixed run for the entire chunk are intentional; do not advance runs per word.
int UiGlyphContext::EmitChunk(std::uint32_t start, int span,
                              const std::uint16_t* words, std::uint32_t count)
{
    UiGlyphRun* run = runs + start;
    std::uint16_t glyphs[32];
    int produced = 0;
    int previous = 0;
    std::uint32_t consumed = 0;
    while (consumed < count) {
        const int length = CallGameMethod<int, 0x00462330>(
            this, words + consumed, count - consumed, run,
            glyphs + produced, &produced, 0, 0);
        const auto nibble = (run->flags >> 12) & 15u;
        const int style = nibble >= 8 ? static_cast<int>(nibble) - 16 : static_cast<int>(nibble);
        CallGameMethod<void, 0x0046C150>(this, start, span, words + consumed,
                                      length, glyphs + previous, produced - previous, style);
        CallGameMethod<void, 0x0046BD90>(this, consumed + start, length);
        consumed += length;
        previous = produced;
    }
    return produced;
}

// 0x0046C480 (__thiscall, ret 8).
void UiGlyphContext::EmitRange(std::uint32_t start, std::uint32_t end)
{
    std::uint16_t words[32];
    std::uint32_t count = start; // reused argument slot in the original
    for (auto index = start; index < end;) {
        const int span = CallGameMethod<int, 0x0046BF10>(this, index, end, words, &count);
        EmitChunk(index, span, words, count);
        index += span;
    }
}
// 0x00526EE0 (__thiscall, ret 12). Key arguments are 32-bit scalar IDs,
// despite the pointer types in the decompiler's provisional signature.
bool UiPropertyTree::Find(std::int32_t innerKey, std::int32_t outerKey, void* output)
{
    UiPropertyNodeHeader* outer;
    CallGameMethod<void, 0x00526490>(this, &outer, &outerKey);
    if (outer == &sentinel) return false;
    auto* nested = reinterpret_cast<UiPropertyTree*>(outer + 1);
    UiPropertyNodeHeader* inner;
    CallGameMethod<void, 0x00526490>(nested, &inner, &innerKey);
    if (inner == &nested->sentinel) return false;
    if (output) *static_cast<void**>(output) = inner + 1;
    return true;
}

// 0x0045F200 (__thiscall, ret 8). The returned borrowed pointer is deliberately
// no longer protected by the lock. Copying does not acquire a reference either.
// The key is the caller's whole 4-byte argument: 0xD38B00 reads and hashes a dword (the original instruction sequence).
// A uint8_t key (first version, #231) truncated it, every lookup above 255 missed and the game hung at the
// main menu (bisected 2026-10-15).
void* UiDescriptorRegistry::Lookup(std::uint32_t key, void* destination)
{
    static_assert(sizeof(UiDescriptorLock) == sizeof(CRITICAL_SECTION));
    auto* section = reinterpret_cast<CRITICAL_SECTION*>(&lock);
    EnterCriticalSection(section);
    // 0xD38B00 returns an 8-byte hash-map iterator {node, bucket} through the pointer (`mov [eax],edx;
    // mov [eax+4],ecx`); the original reserves 8 bytes (`sub esp,8`). A lone pointer here (first version) let it
    // overwrite the next 4 stack bytes, and the game hung at the main menu (bisected 2026-10-15).
    struct { UiDescriptorNode* node; void* bucket; } it;
    CallGameMethod<void, 0x00D38B00>(&mapTag, &it, &key);
    UiDescriptorNode* node = it.node;
    void* result = nullptr;
    if (node != buckets[sentinelIndex]) {
        result = node->descriptor;
        if (destination) {
            std::memcpy(destination, result, 0x278);
            result = destination;
        }
    }
    LeaveCriticalSection(section);
    return result;
}
// 0x0044DA00, cdecl. Only the low byte of create is inspected.
UiDescriptorRegistry* GetDescriptorRegistry(std::uint8_t create)
{
#if defined(_M_IX86)
    auto& cached = *reinterpret_cast<UiDescriptorRegistry**>(0x011B1398);
    if (!cached && create) {
        auto& initialized = *reinterpret_cast<std::uint32_t*>(0x011B1840);
        auto* storage = reinterpret_cast<UiDescriptorRegistry*>(0x011B1720);
        if (!(initialized & 1)) {
            initialized |= 1;
            CallGameMethod<void, 0x0045F7A0>(storage, 0);
            CallGameFunction<int, 0x00F7E9A1>(reinterpret_cast<void(*)()>(0x00F90AF0));
        }
        cached = storage;
    }
    return cached;
#else
    (void)create;
    NotRewritten("descriptor registry globals");
#endif
}

// 0x00466990, thiscall, ret 0. Helpers can alter the visible count and positions.
int UiGlyphContext::Finalize()
{
    if (!layoutState && savedPosition[0] != targetPosition[0]) Rebuild();
    CallGameMethod<void, 0x00462900>(&wordTag);
    const auto visible = visibleCount;
    if (static_cast<std::uint32_t>(words.end - words.begin) != visible) {
        auto* map = static_cast<std::uint32_t*>(wordToGlyph.begin);
        const auto mapped = visible >= (static_cast<std::uint32_t*>(wordToGlyph.end) - map)
            ? static_cast<std::uint32_t>(glyphWords.end - glyphWords.begin) : map[visible];
        glyphWords.Resize(mapped);
        CallGameMethod<void, 0x00465440>(&glyphData, mapped);
        CallGameMethod<void, 0x004654A0>(&metrics, mapped);
        CallGameMethod<void, 0x0044FC30>(&otherData, mapped);
        CallGameMethod<void, 0x00463DA0>(&words, visible);
        CallGameMethod<void, 0x004653C0>(&runs, visible);
        CallGameMethod<void, 0x0044FC30>(&wordToGlyph, visible);
        auto* begin = static_cast<UiLayoutSegment*>(segmentsBegin);
        auto* end = static_cast<UiLayoutSegment*>(segmentsEnd);
        for (auto* segment = begin; segment != end; ++segment) {
            if (segment->end >= visible) {
                segment->end = visible;
                CallGameMethod<void, 0x00465510>(&segmentsBegin, static_cast<std::uint32_t>(segment - begin + 1));
                break;
            }
        }
    }
    finalized = 1;
    CallGameMethod<void, 0x00462D20>(this, visibleCount + prefixCount + suffixCount, currentPosition, 0);
    for (unsigned i = 0; i < 3; ++i) savedPosition[i] = currentPosition[i];
    processedCount += visibleCount;
    if (overflowAllowed && visibleWidth > widthLimit) CallGameMethod<void, 0x00465F90>(this);
    CallGameMethod<void, 0x00465080>(this);
    CallGameMethod<void, 0x00465B50>(this);
    const int result = CallGameMethod<int, 0x004665D0>(this);
    layoutState = 1;
    return result;
}
namespace {
struct TextSpan { const std::uint16_t* words; std::uint32_t count; };
struct TextIterator {
    TextSpan *begin, *end, *span;
    const std::uint16_t* character;
    std::uint32_t first, last, position, scratch, breakKind;
};
TS3_X86_OFFSET(TextIterator, breakKind, 32);
TS3_X86_SIZE(TextIterator, 36);
}
// 0x004635A0, thiscall, ret 0. Empty input leaves all output fields untouched.
void UiGlyphContext::Clip()
{
    const auto count = static_cast<std::uint32_t>(words.end - words.begin);
    if (!count) return;
    TextSpan span{words.begin, count};
    TextIterator iterator;
    CallGameMethod<void, 0x00459880>(&iterator, &span, 1, 0, 0, -1);
    iterator.scratch = iterator.breakKind = 0;
    const float limit = widthLimit;
    const int mode = overflowAllowed ? 0 : (clipMode == 1 ? 4 : clipMode);
    std::uint32_t previous = 0;
    float previousWidth = 0;
    for (;;) {
        const auto boundary = CallGameMethod<std::uint32_t, 0x00451050>(&iterator, 14);
        float width = 0;
        if (boundary) {
            auto* map = static_cast<std::uint32_t*>(wordToGlyph.begin);
            auto glyph = boundary >= (static_cast<std::uint32_t*>(wordToGlyph.end) - map)
                ? static_cast<std::uint32_t>(glyphWords.end - glyphWords.begin) : map[boundary];
            if (glyph) --glyph;
            auto* metric = static_cast<float*>(metrics.begin) + glyph * 8;
            width = metric[3] + metric[1];
        }
        if (width > limit && mode) {
            if (mode == 3) { visibleCount = boundary; visibleWidth = width; }
            else if (mode == 2 || (mode == 4 && !previous)) {
                auto selected = boundary;
                float selectedWidth = width;
                TextIterator characters;
                CallGameMethod<void, 0x00459880>(&characters, &span, 1, 0, 0, -1);
                characters.scratch = 0;
                CallGameMethod<void, 0x004500C0>(&characters, previous);
                // EA leaves visibleCount and visibleWidth alone on this path.
                if (previous >= count) { layoutState = 1; return; }
                for (;;) {
                    const auto next = CallGameMethod<std::uint32_t, 0x00450300>(&characters);
                    const double measured = CallGameMethod<double, 0x00462E80>(this, next);
                    if (measured > limit) break;
                    selected = next;
                    selectedWidth = static_cast<float>(measured);
                }
                visibleCount = selected; visibleWidth = selectedWidth;
            } else {
                if (!previous) {
                    visibleCount = 0; visibleWidth = 0; layoutState = 0; return;
                }
                visibleCount = previous; visibleWidth = previousWidth;
            }
            layoutState = 1; return;
        }
        if (boundary != count && iterator.breakKind == 8) {
            visibleCount = boundary; visibleWidth = width; layoutState = 1; return;
        }
        previous = boundary; previousWidth = width;
        if (boundary >= count) {
            visibleCount = boundary; visibleWidth = width; layoutState = 0; return;
        }
    }
}
// 0x00464DB0, thiscall, ret 0. Callers ignore the incidental EAX residue.
void UiGlyphContext::BuildLayout()
{
    if (words.begin == words.end) return;
    CallGameMethod<void, 0x00462F70>(&wordTag);
    auto count = static_cast<std::uint32_t>(words.end - words.begin);
    std::uint32_t start = 0;
    void* style = runs[0].style;
    // EA advances the index twice at the last word. In particular the last
    // word's style is included in the preceding range even when it changes.
    for (std::uint32_t i = 0; i < count; ++i) {
        void* nextStyle = runs[i].style;
        const bool last = i + 1 == count;
        if (last) ++i;
        if (style != nextStyle || last) {
            SelectFonts(start, i);
            start = i;
            style = nextStyle;
        }
    }
    auto append = [&](UiLayoutSegment value) {
        auto* end = static_cast<UiLayoutSegment*>(segmentsEnd);
        if (reinterpret_cast<std::uintptr_t>(end) < reinterpret_cast<std::uintptr_t>(segmentsCapacity)) {
            segmentsEnd = end + 1;
            if (end) *end = value;
        } else CallGameMethod<void, 0x008A4300>(&segmentsBegin, end, &value);
    };
    count = static_cast<std::uint32_t>(words.end - words.begin);
    if (!segmented) {
        append({30, 0, 0, count});
        EmitRange(0, count);
        return;
    }
    append({-1, 0, 0, 0});
    auto* segment = static_cast<UiLayoutSegment*>(segmentsBegin);
    int currentStyle = FontStyle(runs[0].flags);
    auto variant = [](std::uint32_t flags) {
        const int nibble = (flags >> 12) & 15;
        return nibble < 8 ? nibble : nibble - 16;
    };
    int currentVariant = variant(runs[0].flags);
    start = 0;
    for (std::uint32_t i = 0; i <= count; ++i) {
        const int nextStyle = i < count ? FontStyle(runs[i].flags) : -1;
        const int nextVariant = i < count ? variant(runs[i].flags) : -1;
        const bool changedVariant = nextVariant != currentVariant;
        if (i == count || nextStyle != currentStyle || changedVariant) {
            segment->style = currentStyle;
            segment->variant = currentVariant;
            segment->end = i;
            CallGameMethod<void, 0x00462250>(this, currentStyle, start, i);
            currentVariant = nextVariant;
            start = i;
            if (changedVariant && i != count) {
                append({nextStyle, nextVariant, i, i});
                segment = static_cast<UiLayoutSegment*>(segmentsEnd) - 1;
            }
            currentStyle = nextStyle;
        }
    }
}
// 0x00528660, thiscall, ret 12. The third argument is a pointer, not a float.
double UiWidthContext::Measure(std::int32_t innerKey, std::int32_t outerKey, void* propertyArgument)
{
    auto* property = static_cast<UiWidthProperty*>(propertyArgument);
    float width = defaultWidth;
    if (!property) {
        UiWidthProperty** slot = nullptr;
        if (properties.Find(innerKey, outerKey, &slot)) property = *slot;
    }
    if (property) switch (property->kind) {
    case 1:
        if (property->value) {
            const auto* rectangle = CallGameVirtual<const float*, 14>(property->value);
            width = rectangle[2] - rectangle[0];
        }
        break;
    case 4:
        if (property->value) {
            const auto* integer = static_cast<const std::int32_t*>(property->value);
            width = static_cast<float>(integer[7]);
        }
        break;
    case 2:
        if (property->value) {
            auto* text = static_cast<UiWordVector*>(property->value);
            if (text->end != text->begin) {
                UiTextDescriptor descriptor;
                descriptor.Reset();
                if (CallGameVirtual<bool, 90>(&descriptorVtable, innerKey, outerKey, &descriptor)) {
                    auto* registry = GetDescriptorRegistry(1);
                    if (registry) {
                        auto* style = static_cast<std::uint32_t*>(registry->Lookup(descriptor.fields[0], nullptr));
                        if (style) {
                            const auto saved = style[148];
                            if (!descriptor.mode) style[148] = 0;
                            auto* layout = CallGameMethod<UiGlyphContext*, 0x005137D0>(this);
                            auto* result = CallGameMethod<std::uint8_t*, 0x00462240>(layout);
                            CallGameMethod<void, 0x00463C70>(result, 1);
                            CallGameMethod<void, 0x00465DE0>(layout, 1);
                            CallGameMethod<void, 0x00462CE0>(layout, style);
                            CallGameMethod<void, 0x00464BA0>(layout, text->begin, static_cast<std::uint32_t>(text->end - text->begin));
                            CallGameMethod<void, 0x004621F0>(layout, 500.0f, 0.0f, 0.0f);
                            float maximum = 0;
                            while (CallGameMethod<int, 0x00462230>(layout) != 2) {
                                layout->Finalize();
                                const auto measured = *reinterpret_cast<float*>(result + 184);
                                // EA's COMISS/JBE retains maximum for NaN as well as ties.
                                if (measured > maximum) maximum = measured;
                                CallGameMethod<double, 0x00462CA0>(result);
                                CallGameMethod<void, 0x00463920>(layout, 0.0f);
                            }
                            width = maximum;
                            style[148] = saved;
                        }
                    }
                }
            }
        }
        break;
    }
    // The original uses x87 for these additions: do not round the first sum to float.
    return static_cast<double>(spacing2) + static_cast<double>(spacing1) + static_cast<double>(width);
}
} // namespace ts3::ui
