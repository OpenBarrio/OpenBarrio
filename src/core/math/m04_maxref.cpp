#include "ts3/math/m04_maxref.h"

namespace ts3::math {
const std::int32_t* MaxSignedReference(const std::int32_t* first,
                                     const std::int32_t* second) noexcept
{
    return *first >= *second ? first : second;
}
}
