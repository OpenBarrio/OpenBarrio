#include "ts3/cas/clu_70d020_CAS.h"

namespace ts3::cas {

void CasValueMap::MapValues(int32_t* indices, uint32_t length) const
{
    for (uint32_t i = 0; i < length; ++i) {
        uint32_t found = 0;
        if (count != 0) {
            while (found < count && values[found] != static_cast<uint32_t>(indices[i]))
                ++found;
            if (found == count)
                found = UINT32_MAX;
        } else {
            found = UINT32_MAX;
        }
        indices[i] = static_cast<int32_t>(found);
    }
}

}  // namespace ts3::cas

void __fastcall MapValues(ts3::cas::CasValueMap* self, void*, int32_t* indices, uint32_t length)
{
    self->MapValues(indices, length);
}
