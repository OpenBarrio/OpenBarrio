#include "ts3/resource/m03_G06.h"
#include <string_view>
namespace ts3::resource::m03::g06 {
namespace {
struct Alias { const char16_t* input; const char16_t* output; unsigned address; };
constexpr Alias aliases[]={
    {u"en-us",u"en_us",0x00FD9B2C},
    {u"en-gb",u"en_us",0x00FD9B2C},
    {u"en-au",u"en_us",0x00FD9B2C},
    {u"en-ca",u"en_us",0x00FD9B2C},
    {u"en-nz",u"en_us",0x00FD9B2C},
    {u"is-is",u"en_us",0x00FD9B2C},
    {u"ro-ro",u"en_us",0x00FD9B2C},
    {u"it-it",u"it_it",0x00FD9CD8},
    {u"it-ch",u"it_it",0x00FD9CD8},
    {u"sv-se",u"sv_se",0x00FD9CE4},
    {u"sv-fi",u"sv_se",0x00FD9CE4},
    {u"da-dk",u"da_dk",0x00FD9D68},
    {u"no-no",u"no_no",0x00FD9D5C},
    {u"fi-fi",u"fi_fi",0x00FD9D50},
    {u"hu-hu",u"hu_hu",0x00FD9D44},
    {u"cs-cz",u"cs_cz",0x00FD9D38},
    {u"el-gr",u"el_gr",0x00FD9D2C},
    {u"pt-br",u"pt_br",0x00FD9B38},
    {u"zh-cn",u"zh_cn",0x00FD9AF0},
    {u"zh-sg",u"zh_cn",0x00FD9AF0},
    {u"zh-hk",u"zh_tw",0x00FD9AFC},
    {u"zh-tw",u"zh_tw",0x00FD9AFC},
    {u"ja-jp",u"ja_jp",0x00FD9BE0},
    {u"th-th",u"th_th",0x00FD9BD4},
    {u"nl-nl",u"nl_nl",0x00FD9B08},
    {u"nl-be",u"nl_nl",0x00FD9B08},
    {u"de-de",u"de_de",0x00FD9B14},
    {u"de-at",u"de_de",0x00FD9B14},
    {u"de-ch",u"de_de",0x00FD9B14},
    {u"fr-fr",u"fr_fr",0x00FD9B20},
    {u"fr-be",u"fr_fr",0x00FD9B20},
    {u"fr-ca",u"fr_fr",0x00FD9B20},
    {u"fr-ch",u"fr_fr",0x00FD9B20},
    {u"es-es",u"es_es",0x00FD9B50},
    {u"es-mx",u"es_mx",0x00FD9D20},
    {u"pt-pt",u"pt_pt",0x00FD9D14},
    {u"ko-kr",u"ko_kr",0x00FD9D08},
    {u"ru-ru",u"ru_ru",0x00FD9CFC},
    {u"pl-pl",u"pl_pl",0x00FD9CF0},
};
const char16_t* StaticText(const Alias& a) {
#if defined(_M_IX86)
    return reinterpret_cast<const char16_t*>(a.address);
#else
    return a.output;
#endif
}
}
const char16_t* TableLocale(void* self) {
    const auto input=reinterpret_cast<const char16_t*>(static_cast<const unsigned char*>(self)+0x28);
    const char16_t* end=input;
    while(*end) ++end;
    // Copy and release remain original allocator-family operations. The owned
    // locale buffer is inline UTF16, not a pointer at +28.
    struct Text { const char16_t* begin; const char16_t* end; const char16_t* capacity; const char* name; };
#if defined(_M_IX86)
    auto empty=reinterpret_cast<const char16_t*>(0x011C7E54);
#else
    static constexpr char16_t emptyBuffer[]=u"";
    auto empty=emptyBuffer;
#endif
    Text text{empty,empty,empty+1,"String/LocalizedStringService"};
    ts3::CallGameMethod<void,0x00408E20>(&text,input,end);
    const Alias* found=nullptr;
    const std::u16string_view value(text.begin,static_cast<size_t>(text.end-text.begin));
    for(const auto& a:aliases) {
        if(value==a.input) { found=&a; break; }
    }
    // Only the seven English aliases inline their string destruction. Unknown
    // locales return the same value but take the ordinary destructor path.
    if(found && found->address==0x00FD9B2C) {
        const auto bytes=static_cast<int32_t>((reinterpret_cast<uintptr_t>(text.capacity)-reinterpret_cast<uintptr_t>(text.begin))&~uintptr_t(1));
        if(bytes>2 && text.begin) ts3::CallGameFunction<void,0x004E3F70>(text.begin);
    } else {
        ts3::CallGameMethod<void,0x004EF330>(&text);
    }
    return StaticText(found?*found:aliases[0]);
}
#if defined(_M_IX86)
const char16_t* __fastcall HookTableLocale(void* self,void*) { return TableLocale(self); }
#endif
}
