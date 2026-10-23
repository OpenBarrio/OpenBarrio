// Unit clu_571870_UI (UI): the paged item-list readers, the 48-byte transform-block
// init and the vtable forward. Semantics from the TS3W.exe 1.67.2 listings (the
// original addresses are noted on each function).
#include "ts3/ui/clu_571870_UI.h"

namespace ts3::ui {
namespace {

// The byte span between two vector pointers the way the original computes it: a raw
// 32-bit subtract (wrapping at 2^32 on x86) reinterpreted as signed, then a truncating
// division - exactly what the magic multiplies implement (0x2FC44AA3 with sar 9 for
// 2744, 0x78787879 with sar 7 for 272, both with the add-sign correction).
int32_t ByteSpanDiv(const void* end, const void* begin, int32_t divisor)
{
    const int32_t span = static_cast<int32_t>(reinterpret_cast<uintptr_t>(end) -
                                              reinterpret_cast<uintptr_t>(begin));
    return span / divisor;
}

// The page at index `page`, by byte stride: the original's 32-bit imul wraps at 2^32,
// which uintptr_t reproduces on x86.
const uint8_t* PageAt(const UiPageList* list, uint32_t page)
{
    return reinterpret_cast<const uint8_t*>(list->pagesBegin) +
           static_cast<uintptr_t>(page) * kPageStride;
}

}  // namespace

int32_t UiPageList::CountPages() const
{
    // TS3W.exe 0x00577640: (pagesEnd - pagesBegin) / 2744, signed, truncating.
    return ByteSpanDiv(pagesEnd, pagesBegin, static_cast<int32_t>(kPageStride));
}

int32_t UiPageList::CountItems(uint32_t page) const
{
    // TS3W.exe 0x005776A0 (ret 4): the page index is compared against the count
    // UNSIGNED (jae) - a negative count would let every index through, and the page
    // pointer still wraps its 32-bit imul - then the item vector at page+8/+0xC is
    // spanned the same way as the pages.
    const int32_t count = CountPages();
    if (page >= static_cast<uint32_t>(count))
        return 0;
    const auto* base = reinterpret_cast<const UiPage*>(PageAt(this, page));
    return ByteSpanDiv(base->itemsEnd, base->itemsBegin, static_cast<int32_t>(kItemStride));
}

uint8_t UiPageList::GetItemFields(uint32_t page, uint32_t item, uint32_t* out) const
{
    // TS3W.exe 0x00577700 (ret 0xC): both bounds checks are the same unsigned compares;
    // out[3] takes the row's +0x108 dword and out[4] the +0x104 one (the EA copy order
    // is not monotonic). The success path sets only AL, so the upper EAX bits stay
    // whatever the divisions left - callers test AL, and this returns a clean 1.
    const int32_t pageCount = CountPages();
    if (page >= static_cast<uint32_t>(pageCount))
        return 0;
    const auto* base = reinterpret_cast<const UiPage*>(PageAt(this, page));
    const int32_t itemCount =
        ByteSpanDiv(base->itemsEnd, base->itemsBegin, static_cast<int32_t>(kItemStride));
    if (item >= static_cast<uint32_t>(itemCount))
        return 0;
    const auto* row = reinterpret_cast<const UiItemRow*>(
        reinterpret_cast<const uint8_t*>(base->itemsBegin) +
        static_cast<uintptr_t>(item) * kItemStride);
    out[0] = row->dE0;
    out[1] = row->dE4;
    out[2] = row->dF4;
    out[3] = row->d108;
    out[4] = row->d104;
    return 1;
}

void UiTransformBlock::Init575600()
{
    // TS3W.exe 0x00575600: three movaps rows over `this` - xorps zeros at +0x00, the
    // stack temp {0,0,0,one} at +0x10, and `one` broadcast with shufps at +0x20. The
    // 1.0f is loaded from the float global 0x107A538 (1.0f in TS3W.exe 1.67.2); other
    // code can write it, so the x86 build reads it through the game's memory like the
    // original. Bytes at +0x30 and beyond are never touched.
#if defined(_M_IX86)
    const float one = *reinterpret_cast<const float*>(0x107A538);
#else
    const float one = 1.0f;
#endif
    v[0] = v[1] = v[2] = v[3] = 0.0f;
    v[4] = v[5] = v[6] = 0.0f;
    v[7] = one;
    v[8] = v[9] = v[10] = v[11] = one;
}

int32_t __cdecl Forward571870(void* obj)
{
    // TS3W.exe 0x00571870 (cdecl, plain ret): null in, 0 out; otherwise the argument IS
    // the object - mov ecx, arg / vtable slot 3 (thiscall, one stack argument) / push
    // 0x017F1D88 / call / return its eax. The constant is pushed, never dereferenced
    // here (whatever the callee does with it is the callee's business). x64 builds abort
    // in the layout.h bridge - there is no game code to call.
    if (!obj)
        return 0;
    return CallGameVirtual<int32_t, 3>(obj, reinterpret_cast<const void*>(0x017F1D88));
}

}  // namespace ts3::ui
