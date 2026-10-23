// Portable rewrites for the EATextUtil string helpers.
#include "ts3/eatextutil/clu_4fdce0_EATextUtil.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "ts3/layout.h"

namespace ts3::eatextutil {

char* CopyString004FDCE0(char* destination, const char* source)
{
    // 0x004FDCE0 is a bytewise strcpy, including its terminating NUL.
    char* const originalDestination = destination;
    while ((*destination++ = *source++) != '\0') {}
    return originalDestination;
}

char* LowerAscii004FF500(char* text)
{
    // 0x004FF500 indexes 0x0114CB30 only for signed values 0..127. The
    // game table is ASCII lowercase and leaves punctuation/control bytes intact.
    for (unsigned char* p = reinterpret_cast<unsigned char*>(text); *p; ++p) {
        if (*p <= 0x7f && *p >= 'A' && *p <= 'Z')
            *p = static_cast<unsigned char>(*p + ('a' - 'A'));
    }
    return text;
}

bool MatchWildcard00500080(const char* text, const char* pattern)
{
    // 0x00500080 is deliberately case-sensitive. The caller normalizes both
    // strings on the ordinary path; a nonzero flag bypasses normalization.
    for (;;) {
        const char token = *pattern;
        if (token == '*' && pattern[1] == '\0')
            return true;
        if (*text == '\0')
            return token == '\0';
        if (token == '*') {
            if (MatchWildcard00500080(text, pattern + 1))
                return true;
            ++text;
        } else if (token == '?') {
            ++pattern;
            ++text;
        } else {
            if (*text != token)
                return false;
            ++pattern;
            ++text;
        }
    }
}

namespace {
void* AllocateScratch(std::size_t size, const char* name)
{
#if defined(_M_IX86)
    // The original passes (size, name, 0, 0, 0, 0) at both call sites.
    return CallGameFunction<void*, 0x004E3F90>(size, name, 0, 0, 0, 0);
#else
    (void)name;
    return std::malloc(size);
#endif
}

void FreeScratch(void* memory)
{
#if defined(_M_IX86)
    CallGameFunction<void, 0x004E3F70>(memory);
#else
    std::free(memory);
#endif
}
}  // namespace

bool NormalizeAndMatch0050023F(const char* text, const char* pattern)
{
    // 0x0050023F is reached after 0x00500210 has allocated its 0x1A4 frame.
    // Keep the original stack-vs-EA-heap thresholds (0x17F and 0x1F).
    const std::size_t textLength = std::strlen(text);
    const std::size_t patternLength = std::strlen(pattern);
    char textLocal[0x17f];
    char patternLocal[0x1f];
    char* normalizedText = textLength < sizeof(textLocal)
                               ? textLocal
                               : static_cast<char*>(AllocateScratch(textLength + 1,
                                     "EATextUtil/StringAllocated/char[]"));
    char* normalizedPattern = patternLength < sizeof(patternLocal)
                                  ? patternLocal
                                  : static_cast<char*>(AllocateScratch(patternLength + 1,
                                        "EATextUtil/PatternAllocated/char[]"));
    CopyString004FDCE0(normalizedText, text);
    LowerAscii004FF500(normalizedText);
    CopyString004FDCE0(normalizedPattern, pattern);
    LowerAscii004FF500(normalizedPattern);
    const bool result = MatchWildcard00500080(normalizedText, normalizedPattern);
    if (textLength >= sizeof(textLocal)) FreeScratch(normalizedText);
    if (patternLength >= sizeof(patternLocal)) FreeScratch(normalizedPattern);
    return result;
}

}  // namespace ts3::eatextutil
