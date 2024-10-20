// TypeTraits.hpp

#ifndef TYPE_TRAITS_HPP
#define TYPE_TRAITS_HPP

#include "MksEnums.hpp"
#include <cstdint>
#include <type_traits>

template <PayloadType PT>
struct PayloadTypeTraits;

#define DEFINE_PAYLOAD_TYPE_TRAITS(payload_type_enum, cpp_type) \
    template <>                                                 \
    struct PayloadTypeTraits<payload_type_enum>                 \
    {                                                           \
        using type = cpp_type;                                  \
    };

// Define the mappings
DEFINE_PAYLOAD_TYPE_TRAITS(PayloadType::UINT4, uint8_t)
DEFINE_PAYLOAD_TYPE_TRAITS(PayloadType::UINT8, uint8_t)
DEFINE_PAYLOAD_TYPE_TRAITS(PayloadType::UINT12, uint16_t)
DEFINE_PAYLOAD_TYPE_TRAITS(PayloadType::UINT16, uint16_t)
DEFINE_PAYLOAD_TYPE_TRAITS(PayloadType::UINT24, uint32_t)
DEFINE_PAYLOAD_TYPE_TRAITS(PayloadType::UINT32, uint32_t)
DEFINE_PAYLOAD_TYPE_TRAITS(PayloadType::UINT48, uint64_t)
DEFINE_PAYLOAD_TYPE_TRAITS(PayloadType::INT16, int16_t)
DEFINE_PAYLOAD_TYPE_TRAITS(PayloadType::INT24, int32_t)
DEFINE_PAYLOAD_TYPE_TRAITS(PayloadType::INT32, int32_t)
DEFINE_PAYLOAD_TYPE_TRAITS(PayloadType::INT48, int64_t)

using PayloadVariant = std::variant<
    uint8_t,
    uint16_t,
    uint32_t,
    uint64_t,
    int16_t,
    int32_t,
    int64_t,
    std::monostate>;

// Add other mappings as needed

#endif // TYPE_TRAITS_HPP