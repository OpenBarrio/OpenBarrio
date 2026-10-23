#include "ts3/world/clu_4dbc30_World.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace ts3::world {
namespace {

// The 256-entry {lo, hi} table the original reads at 0x0114DB30 (entry 0 is {0, 0});
// embedded so the hash is identical without touching the game image.
const uint32_t kHash4FA500Table[512] = {
    0x00000000u, 0x00000000u, 0xA9EA3693u, 0x42F0E1EBu, 0x53D46D26u, 0x85E1C3D7u, 0xFA3E5BB5u, 0xC711223Cu,
    0x0E42ECDFu, 0x49336645u, 0xA7A8DA4Cu, 0x0BC387AEu, 0x5D9681F9u, 0xCCD2A592u, 0xF47CB76Au, 0x8E224479u,
    0x1C85D9BEu, 0x9266CC8Au, 0xB56FEF2Du, 0xD0962D61u, 0x4F51B498u, 0x17870F5Du, 0xE6BB820Bu, 0x5577EEB6u,
    0x12C73561u, 0xDB55AACFu, 0xBB2D03F2u, 0x99A54B24u, 0x41135847u, 0x5EB46918u, 0xE8F96ED4u, 0x1C4488F3u,
    0x90E185EFu, 0x663D78FFu, 0x390BB37Cu, 0x24CD9914u, 0xC335E8C9u, 0xE3DCBB28u, 0x6ADFDE5Au, 0xA12C5AC3u,
    0x9EA36930u, 0x2F0E1EBAu, 0x37495FA3u, 0x6DFEFF51u, 0xCD770416u, 0xAAEFDD6Du, 0x649D3285u, 0xE81F3C86u,
    0x8C645C51u, 0xF45BB475u, 0x258E6AC2u, 0xB6AB559Eu, 0xDFB03177u, 0x71BA77A2u, 0x765A07E4u, 0x334A9649u,
    0x8226B08Eu, 0xBD68D230u, 0x2BCC861Du, 0xFF9833DBu, 0xD1F2DDA8u, 0x388911E7u, 0x7818EB3Bu, 0x7A79F00Cu,
    0x21C30BDEu, 0xCC7AF1FFu, 0x88293D4Du, 0x8E8A1014u, 0x721766F8u, 0x499B3228u, 0xDBFD506Bu, 0x0B6BD3C3u,
    0x2F81E701u, 0x854997BAu, 0x866BD192u, 0xC7B97651u, 0x7C558A27u, 0x00A8546Du, 0xD5BFBCB4u, 0x4258B586u,
    0x3D46D260u, 0x5E1C3D75u, 0x94ACE4F3u, 0x1CECDC9Eu, 0x6E92BF46u, 0xDBFDFEA2u, 0xC77889D5u, 0x990D1F49u,
    0x33043EBFu, 0x172F5B30u, 0x9AEE082Cu, 0x55DFBADBu, 0x60D05399u, 0x92CE98E7u, 0xC93A650Au, 0xD03E790Cu,
    0xB1228E31u, 0xAA478900u, 0x18C8B8A2u, 0xE8B768EBu, 0xE2F6E317u, 0x2FA64AD7u, 0x4B1CD584u, 0x6D56AB3Cu,
    0xBF6062EEu, 0xE374EF45u, 0x168A547Du, 0xA1840EAEu, 0xECB40FC8u, 0x66952C92u, 0x455E395Bu, 0x2465CD79u,
    0xADA7578Fu, 0x3821458Au, 0x044D611Cu, 0x7AD1A461u, 0xFE733AA9u, 0xBDC0865Du, 0x57990C3Au, 0xFF3067B6u,
    0xA3E5BB50u, 0x711223CFu, 0x0A0F8DC3u, 0x33E2C224u, 0xF031D676u, 0xF4F3E018u, 0x59DBE0E5u, 0xB60301F3u,
    0xEA6C212Fu, 0xDA050215u, 0x438617BCu, 0x98F5E3FEu, 0xB9B84C09u, 0x5FE4C1C2u, 0x10527A9Au, 0x1D142029u,
    0xE42ECDF0u, 0x93366450u, 0x4DC4FB63u, 0xD1C685BBu, 0xB7FAA0D6u, 0x16D7A787u, 0x1E109645u, 0x5427466Cu,
    0xF6E9F891u, 0x4863CE9Fu, 0x5F03CE02u, 0x0A932F74u, 0xA53D95B7u, 0xCD820D48u, 0x0CD7A324u, 0x8F72ECA3u,
    0xF8AB144Eu, 0x0150A8DAu, 0x514122DDu, 0x43A04931u, 0xAB7F7968u, 0x84B16B0Du, 0x02954FFBu, 0xC6418AE6u,
    0x7A8DA4C0u, 0xBC387AEAu, 0xD3679253u, 0xFEC89B01u, 0x2959C9E6u, 0x39D9B93Du, 0x80B3FF75u, 0x7B2958D6u,
    0x74CF481Fu, 0xF50B1CAFu, 0xDD257E8Cu, 0xB7FBFD44u, 0x271B2539u, 0x70EADF78u, 0x8EF113AAu, 0x321A3E93u,
    0x66087D7Eu, 0x2E5EB660u, 0xCFE24BEDu, 0x6CAE578Bu, 0x35DC1058u, 0xABBF75B7u, 0x9C3626CBu, 0xE94F945Cu,
    0x684A91A1u, 0x676DD025u, 0xC1A0A732u, 0x259D31CEu, 0x3B9EFC87u, 0xE28C13F2u, 0x9274CA14u, 0xA07CF219u,
    0xCBAF2AF1u, 0x167FF3EAu, 0x62451C62u, 0x548F1201u, 0x987B47D7u, 0x939E303Du, 0x31917144u, 0xD16ED1D6u,
    0xC5EDC62Eu, 0x5F4C95AFu, 0x6C07F0BDu, 0x1DBC7444u, 0x9639AB08u, 0xDAAD5678u, 0x3FD39D9Bu, 0x985DB793u,
    0xD72AF34Fu, 0x84193F60u, 0x7EC0C5DCu, 0xC6E9DE8Bu, 0x84FE9E69u, 0x01F8FCB7u, 0x2D14A8FAu, 0x43081D5Cu,
    0xD9681F90u, 0xCD2A5925u, 0x70822903u, 0x8FDAB8CEu, 0x8ABC72B6u, 0x48CB9AF2u, 0x23564425u, 0x0A3B7B19u,
    0x5B4EAF1Eu, 0x70428B15u, 0xF2A4998Du, 0x32B26AFEu, 0x089AC238u, 0xF5A348C2u, 0xA170F4ABu, 0xB753A929u,
    0x550C43C1u, 0x3971ED50u, 0xFCE67552u, 0x7B810CBBu, 0x06D82EE7u, 0xBC902E87u, 0xAF321874u, 0xFE60CF6Cu,
    0x47CB76A0u, 0xE224479Fu, 0xEE214033u, 0xA0D4A674u, 0x141F1B86u, 0x67C58448u, 0xBDF52D15u, 0x253565A3u,
    0x49899A7Fu, 0xAB1721DAu, 0xE063ACECu, 0xE9E7C031u, 0x1A5DF759u, 0x2EF6E20Du, 0xB3B7C1CAu, 0x6C0603E6u,
    0x7D3274CDu, 0xF6FAE5C0u, 0xD4D8425Eu, 0xB40A042Bu, 0x2EE619EBu, 0x731B2617u, 0x870C2F78u, 0x31EBC7FCu,
    0x73709812u, 0xBFC98385u, 0xDA9AAE81u, 0xFD39626Eu, 0x20A4F534u, 0x3A284052u, 0x894EC3A7u, 0x78D8A1B9u,
    0x61B7AD73u, 0x649C294Au, 0xC85D9BE0u, 0x266CC8A1u, 0x3263C055u, 0xE17DEA9Du, 0x9B89F6C6u, 0xA38D0B76u,
    0x6FF541ACu, 0x2DAF4F0Fu, 0xC61F773Fu, 0x6F5FAEE4u, 0x3C212C8Au, 0xA84E8CD8u, 0x95CB1A19u, 0xEABE6D33u,
    0xEDD3F122u, 0x90C79D3Fu, 0x4439C7B1u, 0xD2377CD4u, 0xBE079C04u, 0x15265EE8u, 0x17EDAA97u, 0x57D6BF03u,
    0xE3911DFDu, 0xD9F4FB7Au, 0x4A7B2B6Eu, 0x9B041A91u, 0xB04570DBu, 0x5C1538ADu, 0x19AF4648u, 0x1EE5D946u,
    0xF156289Cu, 0x02A151B5u, 0x58BC1E0Fu, 0x4051B05Eu, 0xA28245BAu, 0x87409262u, 0x0B687329u, 0xC5B07389u,
    0xFF14C443u, 0x4B9237F0u, 0x56FEF2D0u, 0x0962D61Bu, 0xACC0A965u, 0xCE73F427u, 0x052A9FF6u, 0x8C8315CCu,
    0x5CF17F13u, 0x3A80143Fu, 0xF51B4980u, 0x7870F5D4u, 0x0F251235u, 0xBF61D7E8u, 0xA6CF24A6u, 0xFD913603u,
    0x52B393CCu, 0x73B3727Au, 0xFB59A55Fu, 0x31439391u, 0x0167FEEAu, 0xF652B1ADu, 0xA88DC879u, 0xB4A25046u,
    0x4074A6ADu, 0xA8E6D8B5u, 0xE99E903Eu, 0xEA16395Eu, 0x13A0CB8Bu, 0x2D071B62u, 0xBA4AFD18u, 0x6FF7FA89u,
    0x4E364A72u, 0xE1D5BEF0u, 0xE7DC7CE1u, 0xA3255F1Bu, 0x1DE22754u, 0x64347D27u, 0xB40811C7u, 0x26C49CCCu,
    0xCC10FAFCu, 0x5CBD6CC0u, 0x65FACC6Fu, 0x1E4D8D2Bu, 0x9FC497DAu, 0xD95CAF17u, 0x362EA149u, 0x9BAC4EFCu,
    0xC2521623u, 0x158E0A85u, 0x6BB820B0u, 0x577EEB6Eu, 0x91867B05u, 0x906FC952u, 0x386C4D96u, 0xD29F28B9u,
    0xD0952342u, 0xCEDBA04Au, 0x797F15D1u, 0x8C2B41A1u, 0x83414E64u, 0x4B3A639Du, 0x2AAB78F7u, 0x09CA8276u,
    0xDED7CF9Du, 0x87E8C60Fu, 0x773DF90Eu, 0xC51827E4u, 0x8D03A2BBu, 0x020905D8u, 0x24E99428u, 0x40F9E433u,
    0x975E55E2u, 0x2CFFE7D5u, 0x3EB46371u, 0x6E0F063Eu, 0xC48A38C4u, 0xA91E2402u, 0x6D600E57u, 0xEBEEC5E9u,
    0x991CB93Du, 0x65CC8190u, 0x30F68FAEu, 0x273C607Bu, 0xCAC8D41Bu, 0xE02D4247u, 0x6322E288u, 0xA2DDA3ACu,
    0x8BDB8C5Cu, 0xBE992B5Fu, 0x2231BACFu, 0xFC69CAB4u, 0xD80FE17Au, 0x3B78E888u, 0x71E5D7E9u, 0x79880963u,
    0x85996083u, 0xF7AA4D1Au, 0x2C735610u, 0xB55AACF1u, 0xD64D0DA5u, 0x724B8ECDu, 0x7FA73B36u, 0x30BB6F26u,
    0x07BFD00Du, 0x4AC29F2Au, 0xAE55E69Eu, 0x08327EC1u, 0x546BBD2Bu, 0xCF235CFDu, 0xFD818BB8u, 0x8DD3BD16u,
    0x09FD3CD2u, 0x03F1F96Fu, 0xA0170A41u, 0x41011884u, 0x5A2951F4u, 0x86103AB8u, 0xF3C36767u, 0xC4E0DB53u,
    0x1B3A09B3u, 0xD8A453A0u, 0xB2D03F20u, 0x9A54B24Bu, 0x48EE6495u, 0x5D459077u, 0xE1045206u, 0x1FB5719Cu,
    0x1578E56Cu, 0x919735E5u, 0xBC92D3FFu, 0xD367D40Eu, 0x46AC884Au, 0x1476F632u, 0xEF46BED9u, 0x568617D9u,
    0xB69D5E3Cu, 0xE085162Au, 0x1F7768AFu, 0xA275F7C1u, 0xE549331Au, 0x6564D5FDu, 0x4CA30589u, 0x27943416u,
    0xB8DFB2E3u, 0xA9B6706Fu, 0x11358470u, 0xEB469184u, 0xEB0BDFC5u, 0x2C57B3B8u, 0x42E1E956u, 0x6EA75253u,
    0xAA188782u, 0x72E3DAA0u, 0x03F2B111u, 0x30133B4Bu, 0xF9CCEAA4u, 0xF7021977u, 0x5026DC37u, 0xB5F2F89Cu,
    0xA45A6B5Du, 0x3BD0BCE5u, 0x0DB05DCEu, 0x79205D0Eu, 0xF78E067Bu, 0xBE317F32u, 0x5E6430E8u, 0xFCC19ED9u,
    0x267CDBD3u, 0x86B86ED5u, 0x8F96ED40u, 0xC4488F3Eu, 0x75A8B6F5u, 0x0359AD02u, 0xDC428066u, 0x41A94CE9u,
    0x283E370Cu, 0xCF8B0890u, 0x81D4019Fu, 0x8D7BE97Bu, 0x7BEA5A2Au, 0x4A6ACB47u, 0xD2006CB9u, 0x089A2AACu,
    0x3AF9026Du, 0x14DEA25Fu, 0x931334FEu, 0x562E43B4u, 0x692D6F4Bu, 0x913F6188u, 0xC0C759D8u, 0xD3CF8063u,
    0x34BBEEB2u, 0x5DEDC41Au, 0x9D51D821u, 0x1F1D25F1u, 0x676F8394u, 0xD80C07CDu, 0xCE85B507u, 0x9AFCE626u,
};

// Game objects are untyped byte records here: the original field offsets are 32-bit
// relativized, so memcpy-based access keeps the source honest about that.
template <class T>
T ReadField(const void* base, size_t offset)
{
    T value;
    std::memcpy(&value, static_cast<const uint8_t*>(base) + offset, sizeof value);
    return value;
}

template <class T>
void WriteField(void* base, size_t offset, T value)
{
    std::memcpy(static_cast<uint8_t*>(base) + offset, &value, sizeof value);
}

constexpr uint32_t Low32(uint64_t value)
{
    return static_cast<uint32_t>(value & 0xFFFFFFFFu);
}

constexpr uint32_t High32(uint64_t value)
{
    return static_cast<uint32_t>(value >> 32);
}

#if defined(_M_IX86)
// The game's empty wide-string singleton: begin/end point at a NUL and cap is two bytes
// past it, so the inline destructor check (cap - begin = 2) leaves it alone.
constexpr uintptr_t kEmptyWideString = 0x011C7E54;
constexpr uintptr_t kEmptyWideStringCap = 0x011C7E56;

void InitEmptyString(TSString4dbc30* string, const char* allocName)
{
    string->begin = reinterpret_cast<void*>(kEmptyWideString);
    string->end = reinterpret_cast<void*>(kEmptyWideString);
    string->cap = reinterpret_cast<void*>(kEmptyWideStringCap);
    string->allocName = allocName;
}

// The inline record destructor the writer expansion runs itself before 0x004E3F70.
void FreeStringIfOwned(TSString4dbc30* string)
{
    const uint32_t begin = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(string->begin));
    const uint32_t cap = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(string->cap));
    const int32_t span = static_cast<int32_t>((cap - begin) & 0xFFFFFFFEu);
    if (span > 2 && string->begin)
        ts3::CallGameFunction<void, 0x004E3F70>(string->begin);
}

// 0x007D7760-style record destructor used by the early writer exits.
void DestroyString(TSString4dbc30* string)
{
    ts3::CallGameMethod<void, 0x004EF330>(string);
}

// The reference-count-style release through the sub-object at +4.
template <class T>
void ReleaseSubObject4(T* object)
{
    ts3::CallGameVirtual<void, 1>(reinterpret_cast<uint8_t*>(object) + 4);
}
#endif

}  // namespace

int32_t CopyFileDbc30(const wchar_t* existing, const wchar_t* destination, uint8_t failIfExists)
{
    // 0x004DBC30: CopyFileW(existing, destination, failIfExists == 0), normalized to 0/1.
    const BOOL copied = CopyFileW(existing, destination, failIfExists == 0);
    return copied ? 1 : 0;
}

uint64_t Hash4FA500(const uint8_t* data, int32_t count, uint32_t seedLo, uint32_t seedHi,
                    uint8_t invert)
{
    // 0x004FA500: hi ^= table[byte(hi) ^ *p].hi after the 8-bit shift; the table is
    // indexed by (data ^ (seedHi >> 24)) and the two halves advance independently.
    uint32_t lo = seedLo;
    uint32_t hi = seedHi;
    const uint8_t* end = data + count;
    for (const uint8_t* p = data; p < end; ++p) {
        const uint32_t index = (*p ^ (hi >> 24)) & 0xFFu;
        hi = kHash4FA500Table[index * 2 + 1] ^ ((hi << 8) | (lo >> 24));
        lo = kHash4FA500Table[index * 2] ^ (lo << 8);
    }
    if (invert) {
        lo = ~lo;
        hi = ~hi;
    }
    return (static_cast<uint64_t>(hi) << 32) | lo;
}

int32_t LoadSaveWrapper56CD30::Invoke(int32_t a2, int32_t a3)
{
    // 0x0056CD30: 0x004DDD20(*(this+2), a2, a3, *(this+4)) then 0x007D77C0(this, result).
    void* handle = ReadField<void*>(bytes, 8);
    void* extra = ReadField<void*>(bytes, 0x10);
    const uint32_t result = ts3::CallGameFunction<uint32_t, 0x004DDD20>(handle, a2, a3, extra);
    return ts3::CallGameMethod<int32_t, 0x007D77C0>(this, result);
}

int32_t LoadSaveWrapper59F5B0::Invoke(int32_t a2, int32_t a3)
{
    // 0x0059F5B0: 0x004DDAB0(*(this+2), a2, a3) then 0x007D77C0(this, result).
    void* handle = ReadField<void*>(bytes, 8);
    const uint32_t result = ts3::CallGameFunction<uint32_t, 0x004DDAB0>(handle, a2, a3);
    return ts3::CallGameMethod<int32_t, 0x007D77C0>(this, result);
}

int32_t LoadSaveWrapper5FE3A0::Invoke(int32_t a2)
{
    // 0x005FE3A0: 0x004DDA90(*(this+2), a2) then 0x007D77C0(this, result). The game
    // passes the whole argument dword through (mov eax,[esp+4]), so keep all 32 bits.
    void* handle = ReadField<void*>(bytes, 8);
    const uint32_t result = ts3::CallGameFunction<uint32_t, 0x004DDA90>(handle, a2);
    return ts3::CallGameMethod<int32_t, 0x007D77C0>(this, result);
}

int32_t LoadSaveHash5C9280::Hash()
{
    // 0x005C9280: seeds are EAX = EDX = 0xFFFFFFFF; every 0x50-byte record contributes
    // four dwords and two 16-byte blocks, then the object tail is folded in.
    uint64_t hash = 0xFFFFFFFFFFFFFFFFull;
    const uint8_t* record = ReadField<const uint8_t*>(bytes, 0xa4);
    const uint8_t* recordEnd = ReadField<const uint8_t*>(bytes, 0xa8);
    for (; record != recordEnd; record += 0x50) {
        hash = Hash4FA500(record, 0x10, Low32(hash), High32(hash), 0);
        hash = Hash4FA500(record + 0x14, 4, Low32(hash), High32(hash), 0);
        hash = Hash4FA500(record + 0x18, 4, Low32(hash), High32(hash), 0);
        hash = Hash4FA500(record + 0x1C, 4, Low32(hash), High32(hash), 0);
        hash = Hash4FA500(record + 0x20, 4, Low32(hash), High32(hash), 0);
        hash = Hash4FA500(record + 0x30, 0x10, Low32(hash), High32(hash), 0);
        hash = Hash4FA500(record + 0x40, 0x10, Low32(hash), High32(hash), 0);
    }
    hash = Hash4FA500(bytes + 8, 4, Low32(hash), High32(hash), 0);
    hash = Hash4FA500(bytes + 0xC, 4, Low32(hash), High32(hash), 0);
    hash = Hash4FA500(bytes + 0x10, 4, Low32(hash), High32(hash), 0);
    hash = Hash4FA500(bytes + 0x14, 4, Low32(hash), High32(hash), 0);
    hash = Hash4FA500(bytes + 0x1C, 4, Low32(hash), High32(hash), 1);
    return static_cast<int32_t>(Low32(hash));
}

uint8_t LoadSaveLookup5D7090::Lookup(int32_t selector, void* key, void* vector)
{
    // 0x005D7090: only selectors 4 and 8 are accepted. Each 0x74-byte record's object is
    // hashed through vtable slot 3, the bucket chain found at +0x114/+0x118, and the
    // record's sub-vector copied into the caller's vector when the key is present.
    if (selector != 4 && selector != 8)
        return 0;

    uint8_t* record = ReadField<uint8_t*>(bytes, 0xC);
    const uint8_t* recordEnd = ReadField<const uint8_t*>(bytes, 0x10);
    if (record != recordEnd) {
        for (;;) {
            void* object = ReadField<void*>(record, 0);
            const uint32_t hash =
                object ? ts3::CallGameVirtual<uint32_t, 3>(object, static_cast<int32_t>(0xEB54B802u))
                       : 0;
            const uint32_t bucketCount = ReadField<uint32_t>(bytes, 0x118);
            uint32_t* buckets = ReadField<uint32_t*>(bytes, 0x114);
            uint32_t* node = nullptr;
            uint32_t* bucket = buckets + hash % bucketCount;
            for (uint32_t* candidate = reinterpret_cast<uint32_t*>(*bucket); candidate != nullptr;
                 candidate = reinterpret_cast<uint32_t*>(candidate[0x30 / 4])) {
                if (*candidate == hash) {
                    node = candidate;
                    break;
                }
            }
            const uint32_t sentinel = buckets[bucketCount];
            if (node != nullptr && hash != sentinel) {
                uint8_t* subBegin;
                uint8_t* subEnd;
                if (selector == 4) {
                    subBegin = ReadField<uint8_t*>(record, 0x28);
                    subEnd = ReadField<uint8_t*>(record, 0x2C);
                } else {
                    subBegin = ReadField<uint8_t*>(record, 0x4C);
                    subEnd = ReadField<uint8_t*>(record, 0x50);
                }
                if (subBegin != subEnd) {
                    auto** destination = reinterpret_cast<uint8_t**>(vector);
                    for (uint8_t** item = reinterpret_cast<uint8_t**>(subBegin);
                         item != reinterpret_cast<uint8_t**>(subEnd); ++item) {
                        uint8_t* value = *item;
                        if (!value)
                            continue;
                        uint8_t** end = reinterpret_cast<uint8_t**>(destination[1]);
                        uint8_t** cap = reinterpret_cast<uint8_t**>(destination[2]);
                        if (end < cap) {
                            destination[1] = reinterpret_cast<uint8_t*>(end + 1);
                            if (end)
                                *end = value;
                        } else {
                            ts3::CallGameMethod<void, 0x00B7B1E0>(vector, end, &value);
                        }
                    }
                }
            }
            record += 0x74;
            if (record == recordEnd)
                break;
        }
    }

    auto** destination = reinterpret_cast<uint8_t**>(vector);
    if (destination[0] == destination[1])
        return 0;

    // Seed the final chain from the object fields, then mix every recorded value through
    // 0x006057F0 until both halves are all ones; the key pair is complemented on exit.
    uint32_t block[4];
    const uint32_t flag = ReadField<uint32_t>(bytes, 0x2EC);
    if ((flag & 3) != 0 || flag == 0) {
        block[0] = block[1] = block[2] = block[3] = 0;
    } else {
        ts3::CallGameMethod<void, 0x005F23B0>(reinterpret_cast<void*>(flag), block);
    }

    auto* keyWords = reinterpret_cast<uint32_t*>(key);
    uint64_t hash = Hash4FA500(bytes + 0x1F0, 12, keyWords[0], keyWords[1], 0);
    WriteField<uint64_t>(key, 0, hash);
    hash = Hash4FA500(bytes + 0x1B4, 4, Low32(hash), High32(hash), 0);
    WriteField<uint64_t>(key, 0, hash);
    hash = Hash4FA500(reinterpret_cast<const uint8_t*>(block), 16, Low32(hash), High32(hash), 0);
    WriteField<uint64_t>(key, 0, hash);

    uint8_t** item = reinterpret_cast<uint8_t**>(destination[0]);
    uint8_t** const itemEnd = reinterpret_cast<uint8_t**>(destination[1]);
    for (; item != itemEnd; ++item) {
        hash = ts3::CallGameMethod<uint64_t, 0x006057F0>(*item, Low32(hash), High32(hash));
        WriteField<uint64_t>(key, 0, hash);
        // The original bails out without complementing the key when the mixed pair is all
        // ones (0x005D7259 compares (lo & hi) against -1).
        if ((Low32(hash) & High32(hash)) == 0xFFFFFFFFu)
            return 0;
    }
    WriteField<uint32_t>(key, 0, ~Low32(hash));
    WriteField<uint32_t>(key, 4, ~High32(hash));
    return 1;
}

int32_t LoadSaveManagerDestroyNativeWriter742B40(int32_t a1, int32_t a2, int32_t a3)
{
    // 0x00742B40: resolve the manager through 0x007692B0, its context through 0x00AAB380,
    // then look up the two names through vtable slot 0x58 and hand them to the writer.
    void* manager = ts3::CallGameFunction<void*, 0x007692B0>();
    if (!manager)
        return 0;
    void* context = ts3::CallGameFunction<void*, 0x00AAB380>();
    if (!context)
        return 0;
    auto* first = ts3::CallGameVirtual<const wchar_t*, 22>(manager, a1);
    if (!first || !*first)
        return 0;
    auto* second = ts3::CallGameVirtual<const wchar_t*, 22>(manager, a3);
    if (!*first)
        return 0;
    return LoadSaveManagerWriteAb0020(reinterpret_cast<LoadSaveManagerAb0020Self*>(context),
                                      reinterpret_cast<const uint8_t*>(first), a2, second);
}

int32_t BridgeLoadSaveManagerDestroyNativeWriter742CB0(int32_t a1, int32_t a2, int32_t a3)
{
    // 0x00742CB0 is a five-byte tail jump to 0x00742B40.
    return LoadSaveManagerDestroyNativeWriter742B40(a1, a2, a3);
}

int32_t LoadSaveWriter7D84E0::Write(int32_t a1, int32_t a2)
{
    // 0x007D84E0: fold 0x004DDBD0's low byte into +0x14, then run 0x004DDAB0 and fold
    // its low byte in as well; the second call's full result is returned.
    void* handle = ReadField<void*>(bytes, 8);
    void* extra = ReadField<void*>(bytes, 0x10);
    const uint32_t first = ts3::CallGameFunction<uint32_t, 0x004DDBD0>(handle, a2, extra);
    bytes[0x14] = static_cast<uint8_t>(bytes[0x14] & static_cast<uint8_t>(first));
    const uint32_t result = ts3::CallGameFunction<uint32_t, 0x004DDAB0>(handle, a1, a2);
    bytes[0x14] = static_cast<uint8_t>(bytes[0x14] & static_cast<uint8_t>(result));
    return static_cast<int32_t>(result);
}

int32_t LoadSaveWriter7D8520::Write(int32_t a1, int32_t a2)
{
    // 0x007D8520: same first step; the second step (0x004DDB00) only runs while the
    // folded flag is still non-zero.
    void* handle = ReadField<void*>(bytes, 8);
    void* extra = ReadField<void*>(bytes, 0x10);
    uint32_t result = ts3::CallGameFunction<uint32_t, 0x004DDBD0>(handle, a2, extra);
    bytes[0x14] = static_cast<uint8_t>(bytes[0x14] & static_cast<uint8_t>(result));
    if (bytes[0x14] != 0) {
        result = ts3::CallGameFunction<uint32_t, 0x004DDB00>(handle, a1, a2, extra);
        bytes[0x14] = static_cast<uint8_t>(bytes[0x14] & static_cast<uint8_t>(result));
    }
    return static_cast<int32_t>(result);
}

namespace {

int32_t CharacterCount(const void* begin, const void* end)
{
    return static_cast<int32_t>((static_cast<const uint8_t*>(end) - static_cast<const uint8_t*>(begin)) / 2);
}

void ReserveString(TSString4dbc30* string, int32_t characters)
{
    ts3::CallGameMethod<void, 0x00408830>(string, characters);
}

void AppendString(TSString4dbc30* string, const void* begin, const void* end)
{
    ts3::CallGameMethod<void, 0x00408B90>(string, begin, end);
}

}  // namespace

TSString4dbc30* Concat7D9C50(TSString4dbc30* out, const TSString4dbc30* a,
                             const TSString4dbc30* b)
{
    // 0x007D9C50: reserve a+b+1 characters, terminate, then append a and b in order.
    const int32_t length = CharacterCount(a->begin, a->end) + CharacterCount(b->begin, b->end);
    out->begin = nullptr;
    out->end = nullptr;
    out->cap = nullptr;
    out->allocName = a->allocName;
    ReserveString(out, length + 1);
    *static_cast<uint16_t*>(out->end) = 0;
    AppendString(out, a->begin, a->end);
    AppendString(out, b->begin, b->end);
    return out;
}

TSString4dbc30* ConcatAppend7D9CC0(TSString4dbc30* out, const TSString4dbc30* a,
                                   const wchar_t* src)
{
    // 0x007D9CC0: same, plus a NUL-terminated wide string whose length is scanned first.
    const wchar_t* scan = src;
    while (*scan)
        ++scan;
    const int32_t extra = static_cast<int32_t>(scan - src);
    const int32_t length = CharacterCount(a->begin, a->end) + extra;
    out->begin = nullptr;
    out->end = nullptr;
    out->cap = nullptr;
    out->allocName = a->allocName;
    ReserveString(out, length + 1);
    *static_cast<uint16_t*>(out->end) = 0;
    AppendString(out, a->begin, a->end);
    AppendString(out, src, scan);
    return out;
}

int32_t LoadSaveManagerWriteAb0020(LoadSaveManagerAb0020Self* self, const uint8_t* data,
                                   int32_t count, const wchar_t* name)
{
#if defined(_M_IX86)
    // 0x00AB0020: the LoadSaveManager native writer. It formats the caller's name, builds
    // the "ChallengeData.package" path and a ".backup" sibling, optionally copies the
    // previous file aside, opens the DBPF writer (0x004E41F0), feeds it the caller buffer
    // and returns one of the 0/4/9/10/15/17 status codes.
    uint8_t node[80];
    ts3::CallGameMethod<void, 0x004EC290>(node);
    void* manager = ts3::CallGameFunction<void*, 0x0072F1C0>();
    uint16_t* managerName = ts3::CallGameVirtual<uint16_t*, 9>(manager);
    if (!managerName || !*managerName)
        return 4;
    ts3::CallGameMethod<void, 0x004ED810>(node, managerName);
    const wchar_t* probe = ts3::CallGameMethod<const wchar_t*, 0x00F626F0>(node);
    if (!ts3::CallGameFunction<uint8_t, 0x004DCFF0>(probe))
        return 4;

    void* packageManager = ts3::CallGameFunction<void*, 0x00984550>();

    TSString4dbc30 package;
    package.begin = nullptr;
    package.end = nullptr;
    package.cap = nullptr;
    package.allocName = "World/LoadSaveManager";
    ts3::CallGameMethod<int32_t, 0x0040B700>(&package, L"ChallengeData.package");

    TSString4dbc30 fileName;
    fileName.begin = nullptr;
    fileName.end = nullptr;
    fileName.cap = nullptr;
    fileName.allocName = "World/LoadSaveManager";
    ts3::CallGameMethod<int32_t, 0x0040B700>(&fileName,
                                             ReadField<void*>(packageManager, 0x100));

    const int64_t formatted = ts3::CallGameFunction<int64_t, 0x007CFE40>(name);
    char buffer[64];
    std::sprintf(buffer, "%I64d", formatted);

    TSString4dbc30 temp;
    InitEmptyString(&temp, "World/LoadSaveManager");

    TSString4dbc30 existing = {};
    TSString4dbc30* result =
        ts3::CallGameFunction<TSString4dbc30*, 0x004F3750>(&existing, buffer, -1, 0);
    if (result != &temp)
        ts3::CallGameMethod<void, 0x00408E20>(&temp, result->begin, result->end);
    FreeStringIfOwned(&existing);

    TSString4dbc30 concat;
    TSString4dbc30* combined = Concat7D9C50(&concat, &fileName, &temp);
    TSString4dbc30* backup = ConcatAppend7D9CC0(&existing, combined, L"\\");
    if (backup != &fileName)
        ts3::CallGameMethod<void, 0x00408E20>(&fileName, backup->begin, backup->end);
    FreeStringIfOwned(&existing);
    FreeStringIfOwned(&concat);

    const wchar_t* current = static_cast<const wchar_t*>(fileName.begin);
    ts3::CallGameFunction<int32_t, 0x004DCFF0>(fileName.begin);
    TSString4dbc30* challenge = Concat7D9C50(&concat, &fileName, &package);
    if (challenge != &fileName) {
        ts3::CallGameMethod<void, 0x00408E20>(&fileName, challenge->begin, challenge->end);
        current = static_cast<const wchar_t*>(fileName.begin);
    }
    FreeStringIfOwned(&concat);

    // The block later used as the writer object starts life as a string record.
    alignas(8) uint8_t writerBytes[0x18] = {};
    InitEmptyString(&existing, "World/LoadSaveManager");
    const wchar_t* backupName = reinterpret_cast<const wchar_t*>(kEmptyWideString);
    TSString4dbc30* numbered = Concat7D9C50(reinterpret_cast<TSString4dbc30*>(writerBytes),
                                            &fileName, &existing);
    TSString4dbc30* backupPath = ConcatAppend7D9CC0(&concat, numbered, L".backup");
    if (backupPath != &existing) {
        ts3::CallGameMethod<void, 0x00408E20>(&existing, backupPath->begin, backupPath->end);
        backupName = static_cast<const wchar_t*>(existing.begin);
    }
    FreeStringIfOwned(&concat);
    FreeStringIfOwned(reinterpret_cast<TSString4dbc30*>(writerBytes));

    bool backupMade = false;
    if (ts3::CallGameFunction<int32_t, 0x004DBBA0>(current)) {
        backupMade = true;
        CopyFileDbc30(current, backupName, 1);
    }

    LoadSaveManagerAb0020Self* writer = nullptr;
    int32_t status = 0;
    void* dbpf = ts3::CallGameFunction<void*, 0x004E41F0>(
        920, "World/LoadSaveManager/DBPF", 0, 0, 0, 0);
    if (dbpf) {
        writer = reinterpret_cast<LoadSaveManagerAb0020Self*>(
            ts3::CallGameMethod<void*, 0x004A8F70>(dbpf, fileName.begin, 0));
        if (writer) {
            ReleaseSubObject4(writer);
        }
        backupName = static_cast<const wchar_t*>(existing.begin);
    }

    if (writer && ts3::CallGameVirtual<uint8_t, 1>(writer)) {
        ts3::CallGameMethod<void, 0x005169D0>(writer, 1);
        if (ts3::CallGameVirtual<uint8_t, 6>(writer, 2, 2, 0)) {
            ts3::CallGameMethod<void, 0x007D76F0>(writerBytes, writer, 1);
            if (!ts3::CallGameMethod<uint8_t, 0x007D8290>(writerBytes,
                                                          reinterpret_cast<void*>(0x01182E30), 1)) {
                ts3::CallGameMethod<void, 0x007D7760>(writerBytes);
                ReleaseSubObject4(writer);
                DestroyString(&existing);
                DestroyString(&temp);
                DestroyString(&fileName);
                DestroyString(&package);
                return 9;
            }

            reinterpret_cast<LoadSaveWrapper5FE3A0*>(writerBytes)->Invoke(1);
            char version[4] = {'S', '3', '\0', '\0'};
            reinterpret_cast<LoadSaveWrapper59F5B0*>(writerBytes)->Invoke(
                static_cast<int32_t>(reinterpret_cast<uintptr_t>(version)), 3);
            ts3::CallGameMethod<void, 0x00565730>(writerBytes, 4);
            reinterpret_cast<LoadSaveWrapper5FE3A0*>(writerBytes)->Invoke(4);
            const int32_t firstBegin = ReadField<int32_t>(self, 0x34);
            const int32_t firstEnd = ReadField<int32_t>(self, 0x38);
            const int32_t secondBegin = ReadField<int32_t>(self, 0x44);
            const int32_t secondEnd = ReadField<int32_t>(self, 0x48);
            reinterpret_cast<LoadSaveWriter7D84E0*>(writerBytes)->Write(firstBegin,
                                                                        firstEnd - firstBegin);
            reinterpret_cast<LoadSaveWriter7D84E0*>(writerBytes)->Write(secondBegin,
                                                                        secondEnd - secondBegin);
            const uint64_t hash = Hash4FA500(data, 2 * count, 0xFFFFFFFFu, 0xFFFFFFFFu, 1);
            reinterpret_cast<LoadSaveWrapper56CD30*>(writerBytes)->Invoke(
                static_cast<int32_t>(Low32(hash)), static_cast<int32_t>(High32(hash)));
            reinterpret_cast<LoadSaveWriter7D8520*>(writerBytes)->Write(
                static_cast<int32_t>(reinterpret_cast<uintptr_t>(data)), count);
            ts3::CallGameMethod<void, 0x007D7840>(writerBytes);
            if (!writerBytes[0x14]) {
                ts3::CallGameMethod<void, 0x007D7760>(writerBytes);
                ReleaseSubObject4(writer);
                DestroyString(&existing);
                DestroyString(&temp);
                DestroyString(&fileName);
                DestroyString(&package);
                return 10;
            }
            ts3::CallGameVirtual<void, 7>(writer);
            ts3::CallGameMethod<void, 0x007D7760>(writerBytes);
            backupName = static_cast<const wchar_t*>(existing.begin);
        } else {
            status = 15;
        }
        ts3::CallGameVirtual<void, 2>(writer);
        ReleaseSubObject4(writer);
        writer = nullptr;
    } else {
        status = 17;
    }

    // On a non-zero status the previous file is restored from the .backup sibling.
    if (status != 0 && backupMade) {
        const wchar_t* fileNameBegin = static_cast<const wchar_t*>(fileName.begin);
        ts3::CallGameFunction<int32_t, 0x004DBBF0>(backupName, fileNameBegin, 1);
        ts3::CallGameFunction<int32_t, 0x00AAF4B0>(fileNameBegin);
    }

    if (ts3::CallGameFunction<int32_t, 0x004DBBA0>(existing.begin))
        ts3::CallGameFunction<int32_t, 0x004DBBD0>(existing.begin);
    if (writer)
        ReleaseSubObject4(writer);
    FreeStringIfOwned(&existing);
    FreeStringIfOwned(&temp);
    FreeStringIfOwned(&fileName);
    FreeStringIfOwned(&package);
    return status;
#else
    (void)self;
    (void)data;
    (void)count;
    (void)name;
    ts3::AbortNotRewritten("LoadSaveManager native writer 0x00AB0020");
    return 0;
#endif
}

}  // namespace ts3::world