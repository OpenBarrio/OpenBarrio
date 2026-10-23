#include "ts3/ui/clu_44d890_UI.h"
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <limits>
#include <cwctype>
#include "ts3/io/memory_stream.h"

namespace ts3::ui44d890 {
double ParseDouble(const char* text) // 0x0044DA50
{
    // VC2005 strtod accepts decimal syntax only. New CRTs additionally accept
    // infinities, NaNs and hexadecimal floats, which change the game's outputs.
    const char* first = text;
    while (std::isspace(static_cast<unsigned char>(*first))) ++first;
    const bool negative = *first == '-';
    if (*first == '+' || *first == '-') ++first;
    if ((*first < '0' || *first > '9') && *first != '.') return 0.0;
    if (first[0] == '0' && (first[1] == 'x' || first[1] == 'X'))
        return negative ? -0.0 : 0.0;
    return std::strtod(text, nullptr);
}
uint32_t ParseUnsigned(const char* text) // 0x00453040
{
    return static_cast<uint32_t>(std::strtoul(text, nullptr, 10));
}
uint32_t EffectNameHash(const char* text) // 0x0045F0F0
{
    if (!text || !*text) return 0;
    uint32_t hash = 0x811c9dc5;
    do { hash = (hash * 0x01000193) ^ static_cast<uint8_t>(*text++); } while (*text);
    return hash;
}
int CompareAscii(const char* left, const char* right) // 0x00470A60
{
    for (;;) {
        uint8_t a = static_cast<uint8_t>(*left++), b = static_cast<uint8_t>(*right++);
        if (static_cast<uint8_t>(a - 'A') <= 25) a += 32;
        if (static_cast<uint8_t>(b - 'A') <= 25) b += 32;
        if (a != b) return static_cast<int8_t>(a) - static_cast<int8_t>(b);
        if (!a) return 0;
    }
}
bool ReadToken(const char* input, uint32_t length, char delimiter, char* output,
               uint32_t capacity, const char** next) // 0x00470D60
{
    if (output && capacity) *output = 0;
    bool found = false, trailing = false;
    if (!input) return false;
    uint32_t written = 1;
    for (uint32_t i = 0; i < length && input[i]; ++i) {
        if (input[i] == delimiter) {
            if (found) trailing = true;
        } else {
            found = true;
            if (trailing) return true;
            if (output && written < capacity) {
                *output++ = input[i]; ++written; *output = 0;
            }
        }
        if (next) ++*next;
    }
    return found;
}
void Buffer::Reset() // 0x00467D30 (EAX is unspecified; callers ignore it)
{
    if (allocator) CallGameVirtual<void, 3>(allocator, begin, size_t{0});
    begin = nullptr;
    size = capacity = field0c = field10 = 0;
}
namespace {
void*& GlobalPointer(uint32_t address)
{
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(static_cast<uintptr_t>(address));
#else
    (void)address;
    NotRewritten("UI font server global");
#endif
}
}
void* GetFontServer(bool create) // 0x0044D890
{
    void*& server = GlobalPointer(0x011b138c);
    if (!server && create) {
        void*& storage = GlobalPointer(0x011b1390);
        if (!storage) {
            void*& allocator = GlobalPointer(0x011b1380);
            if (!allocator) allocator = CallGameFunction<void*, 0x00931020>();
            void* block = CallGameVirtual<void*, 2>(allocator, size_t{0x5338}, static_cast<const char*>(nullptr), 0u);
            storage = block ? CallGameMethod<void*, 0x004592b0>(block, 0) : nullptr;
        }
        server = storage;
        // EA dereferences null when allocation fails; preserve that behavior.
        CallGameVirtual<void, 1>(storage);
    }
    return server;
}
bool InitializeFontServer() // 0x009EF0C0
{
    GetFontServer(true);
    return true;
}
void* EnsureFontServer() // 0x009EF1B0
{
    return GetFontServer(true);
}
// 00453160: original ignores capacity completely; preserve this EA buffer-overflow bug.
int ParseEffect(const char* script, std::uint32_t* output,
                     int capacity, int* errorLine)
{
    (void)capacity;
    const char* const end = script + std::strlen(script);
    const char* next = script;
    int used = 0;
    int error = -1;
    int lineNumber = 1;
    struct Command { const char* name; int minimum; };
    static const Command commands[] = {
        {"SetExpansion", 3}, {"SetGlyphMinAlpha", 2}, {"SetGlyphColor", 2},
        {"SetGlyphOffset", 3}, {"SetGlyphHSpace", 3}, {"SetGlyphVSpace", 3},
        {"SetGlyphSmooth", 2}, {"SetGlyphBrush", 5}, {"SetTargetImage", 2},
        {"SetTargetMask", 2}, {"DrawGlyphSmearOutline", 1},
        {"DrawGlyphBrushOutline", 1}, {"DrawGlyph", 1}, {"Clear", 1},
        {"ApplyCurve", 3}, {"ApplyBoxBlur", 4}, {"Merge", 1}, {"CallUser", 2}
    };
    auto emit = [&](std::uint32_t value) { output[used++] = value; };
    auto emitFloat = [&](double value) {
        const float rounded = static_cast<float>(value);
        std::uint32_t bits;
        std::memcpy(&bits, &rounded, sizeof(bits));
        emit(bits);
    };
    do {
        const char* tokenCursor = next;
        const char* lineEnd = ts3::CallGameFunction<const char*, 0x00470C80>(next, end, &next);
        char tokens[6][32]{};
        int count = 0;
        while (count < 6 && ReadToken(tokenCursor,
                 static_cast<std::uint32_t>(lineEnd - tokenCursor), ' ',
                 tokens[count], 32u, &tokenCursor)) ++count;
        if (count) {
            int op = 0;
            for (int i = 0; i < 18; ++i) {
                if (!CompareAscii(tokens[0], commands[i].name)) { op = i + 1; break; }
            }
            if (!op) {
                if (std::strstr(tokens[0], "//") != tokens[0]) error = lineNumber;
            } else {
                if (count < commands[op - 1].minimum) error = lineNumber;
                // Emit even on missing arguments. Zero-filled absent tokens parse as zero.
                emit(static_cast<std::uint32_t>(op));
                auto unsignedAt = [&](int i, int base = 10) {
                    return static_cast<std::uint32_t>(std::strtoul(tokens[i], nullptr, base));
                };
                auto signedAt = [&](int i) {
                    return static_cast<std::uint32_t>(std::strtol(tokens[i], nullptr, 10));
                };
                switch (op) {
                case 1: emit(unsignedAt(1)); emit(unsignedAt(2)); break;
                case 2: case 9: emit(unsignedAt(1)); break;
                case 3: emit(unsignedAt(1, 16)); break;
                case 4: case 5: case 6: emit(signedAt(1)); emit(signedAt(2)); break;
                case 7: emit(unsignedAt(1) != 0); break;
                case 8:
                    emit(unsignedAt(1)); emit(unsignedAt(2));
                    emitFloat(ParseDouble(tokens[3]));
                    emitFloat(ParseDouble(tokens[4])); break;
                case 10: emit(ParseUnsigned(tokens[1])); break;
                case 15:
                    emit(ParseUnsigned(tokens[1])); emitFloat(ParseDouble(tokens[2])); break;
                case 16:
                    emitFloat(ParseDouble(tokens[1])); emitFloat(ParseDouble(tokens[2]));
                    emit(ParseUnsigned(tokens[3])); break;
                case 18: emit(unsignedAt(1, 0)); break;
                default: break;
                }
            }
        }
        ++lineNumber;
    } while (next != end && error == -1);
    if (errorLine) *errorLine = error;
    return error == -1 ? used : 0;
}

// 00454560: settings is 0xE4 bytes, all fixed-width words, no pointer fields.
// Header words[0..15], program words[16..55], programLength words[56].
int InitializeEffect(const char* script, std::uint32_t* settings, int* errorLine)
{
    settings[56] = 0;
    for (int i = 2; i <= 7; ++i) settings[i] = 0;
    settings[10] = 0;
    settings[11] = 3;
    settings[15] = 3;
    settings[9] = 1;
    settings[14] = 1;
    #if defined(_M_IX86)
    settings[12] = *reinterpret_cast<const uint32_t*>(0x00f9a5ac);
#else
    settings[12] = 0x3F000000;
#endif
    settings[0] = 2;
    settings[1] = 0xFF000000;
    settings[8] = 12;
    #if defined(_M_IX86)
    settings[13] = *reinterpret_cast<const uint32_t*>(0x0107a538);
#else
    settings[13] = 0x3F800000;
#endif
    const int result = ParseEffect(script, settings + 16, 40, errorLine);
    settings[56] = static_cast<std::uint32_t>(result);
    return result;
}

// ABI: 691D20 = void __cdecl(uint32_t, const char16_t*);
//      691DE0 = void __cdecl(int32_t, const char16_t* const*).
// EAX after 691D20 is indeterminate (destructor return); treat as void.
// Both Hex-Rays st0 double arguments are false recovery artifacts.
struct ConvertedString {
    char* begin; char* end; char* capacity; void* allocator;
};
TS3_X86_SIZE(ConvertedString, 16);
using EffectArray = Buffer;
TS3_X86_SIZE(EffectArray,24);
struct EffectState {
    uint32_t header[16];
    uint8_t commands[160];
    uint32_t commandCount;
    void* commandData;
    uint32_t fieldE8;
    uint8_t remaining[0x200]; // +ec..+2eb
    uint32_t field2ec;
    EffectArray arrays[3];
    uint8_t tail[0x34];
};
TS3_X86_OFFSET(EffectState,commandCount,0xe0);
TS3_X86_OFFSET(EffectState,arrays,0x2f0);
TS3_X86_SIZE(EffectState,0x36c);
struct FontStream {
    ts3::io::MemoryStream base;
    uint32_t field28;
    void* allocator;
};
TS3_X86_OFFSET(FontStream,allocator,0x2c);
TS3_X86_SIZE(FontStream,0x30);
struct FontKey { uint64_t instance; uint32_t type, group; };
TS3_X86_SIZE(FontKey,16);

// 00691D20
void RegisterEffect(uint32_t id, const char16_t* source) {
    EffectState state; // preserve constructor's intentionally uninitialized padding
    ts3::CallGameMethod<void*,0x4545F0>(&state,static_cast<void*>(nullptr));
    int consumed=0;
    ConvertedString converted;
    auto* result=ts3::CallGameFunction<ConvertedString*,0x4F38C0>(&converted,source,-1,0);
    const int parsed=InitializeEffect(result->begin,state.header,&consumed); // local454560
    // Compare signed capacity byte-distance: original intentionally uses signed jle.
    const auto span=static_cast<intptr_t>(reinterpret_cast<uintptr_t>(converted.capacity)-reinterpret_cast<uintptr_t>(converted.begin));
    if(span>1 && converted.begin) ts3::CallGameFunction<void,0x4E3F70>(converted.begin);
    if(parsed) {
        if(auto* server=GetFontServer(0)) // local44d890
            ts3::CallGameMethod<void,0x456800>(server,id,state.commands,state.commandCount);
    }
    state.arrays[2].Reset(); // local467d30
    state.arrays[1].Reset();
    state.arrays[0].Reset();
}

uint32_t ParseHexUnsigned16(const char16_t* text);
// 00691DE0
void FontCommand(int32_t argc, const char16_t* const* argv) {
    if(argc<=0) return;
    auto equal=[](const char16_t* a,const char16_t* b) {
        while(*a==*b) { if(!*a) return true; ++a; ++b; } return false;
    };
    if(equal(argv[0],u"font")) {
        void* server=GetFontServer(0);
        if(!server || argc<=1) return;
        FontKey key { ts3::CallGameFunction<uint64_t,0x7CFE40>(argv[1]),0x062e9ee0,0 };
        void* manager=ts3::CallGameFunction<void*,0x4AFD20>();
        if(!manager) return;
        void* provider=ts3::CallGameVirtual<void*,17>(manager,&key);
        if(!provider) return;
        void* resource=nullptr;
        void** out=ts3::CallGameMethod<void**,0x5C9C90>(&resource);
        // 5C9C90 takes NO stack arguments. The four pre-pushed constants in asm
        // are arguments for this following virtual call, which takes six args.
        if(ts3::CallGameVirtual<bool,13>(provider,&key,out,1,6,1,0)) {
            void* source=ts3::CallGameVirtual<void*,6>(resource);
            if(source) {
                void* allocator=ts3::CallGameFunction<void*,0x44D800>();
                uint32_t length=ts3::CallGameVirtual<uint32_t,7>(source);
                void* data=ts3::CallGameVirtual<void*,2>(allocator,length,"Renderer/FontData",0);
                if(data) {
                    uint32_t read=ts3::CallGameVirtual<uint32_t,12>(source,data,length);
                    void* streamAllocator=ts3::CallGameFunction<void*,0x44D800>();
                    auto* raw=ts3::CallGameVirtual<FontStream*,1>(streamAllocator,static_cast<uint32_t>(sizeof(FontStream)),"UI/EAText/MemoryStream",0,4,0);
                    if(raw) {
                        auto* stream=ts3::CallGameMethod<FontStream*,0x455A70>(raw,data,read,1,1,allocator);
                        if(stream) {
                            ts3::CallGameVirtual<void,1>(stream);
                            stream->allocator=allocator;
                            ts3::CallGameVirtual<void,12>(server,stream,2);
                            ts3::CallGameVirtual<void,2>(stream);
                        }
                    }
                    // EA behavior retained: data is leaked if stream allocation fails.
                }
            }
            ts3::CallGameVirtual<void,8>(resource);
        }
        if(resource) ts3::CallGameVirtual<void,2>(resource);
    } else if(equal(argv[0],u"effectID")) {
        if(argc>2) {
            // Windows wchar_t == char16_t; portable hex conversion needs precise
            // wcstoul semantics (spaces, +/- signs, 0x, saturation 0xffffffff).
            uint32_t id=ParseHexUnsigned16(argv[1]);
            RegisterEffect(id,argv[2]);
        }
    } else if(equal(argv[0],u"effectName") && argc>2) {
        ConvertedString converted;
        auto* result=ts3::CallGameFunction<ConvertedString*,0x4F38C0>(&converted,argv[1],-1,0);
        uint32_t id=EffectNameHash(result->begin); // local45f0f0
        ts3::CallGameMethod<void,0x405940>(&converted);
        RegisterEffect(id,argv[2]);
    }
}
// Mirrors 32-bit CRT wcstoul(base16) numeric results;
// errno is unobservable here, but overflow saturates before sign application.
uint32_t ParseHexUnsigned16(const char16_t* text) {
    while(std::iswspace(static_cast<wint_t>(*text))) ++text;
    bool negative=false;
    if(*text==u'-' || *text==u'+') { negative=*text==u'-'; ++text; }
    auto digit=[](char16_t c)->int {
        if(c>=u'0' && c<=u'9') return c-u'0';
        if(c>=u'a' && c<=u'f') return c-u'a'+10;
        if(c>=u'A' && c<=u'F') return c-u'A'+10;
        return -1;
    };
    if(text[0]==u'0' && (text[1]==u'x' || text[1]==u'X')) text+=2;
    uint32_t value=0;
    bool overflow=false;
    for(int d=digit(*text); d>=0; d=digit(*++text)) {
        if(value>(UINT32_MAX-static_cast<uint32_t>(d))/16) overflow=true;
        value=value*16+static_cast<uint32_t>(d);
    }
    if(overflow) return UINT32_MAX;
    return negative ? 0u-value : value;
}

}
