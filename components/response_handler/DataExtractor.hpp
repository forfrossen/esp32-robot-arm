#ifndef DATA_EXTRACTOR_HPP
#define DATA_EXTRACTOR_HPP

#include "MksEnums.hpp"
#include "TypeTraits.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>

extern uint64_t extract_bits(const uint8_t *data, size_t bit_offset, size_t bit_length);

extern uint64_t extract_data(PayloadType pt, const uint8_t *data, size_t bit_offset);

#endif // DATA_EXTRACTOR_HPP