#include "DataExtractor.hpp"
uint64_t extract_bits(const uint8_t *data, size_t bit_offset, size_t bit_length)
{

    uint64_t value = 0;

    size_t byte_offset = bit_offset / 8;
    size_t bit_in_byte = bit_offset % 8;

    size_t bits_remaining = bit_length;

    while (bits_remaining > 0)
    {
        size_t bits_in_this_byte = std::min(8 - bit_in_byte, bits_remaining);

        uint8_t byte = data[byte_offset];
        uint8_t mask = ((1 << bits_in_this_byte) - 1) << (8 - bit_in_byte - bits_in_this_byte);
        uint8_t extracted_bits = (byte & mask) >> (8 - bit_in_byte - bits_in_this_byte);

        value = (value << bits_in_this_byte) | extracted_bits;

        bits_remaining -= bits_in_this_byte;
        bit_offset += bits_in_this_byte;
        byte_offset = bit_offset / 8;
        bit_in_byte = bit_offset % 8;
    }

    return value;
}

uint64_t extract_data(PayloadType pt, const uint8_t *data, size_t bit_offset)
{
    size_t bit_length = get_payload_type_size(pt);
    uint64_t raw_value = extract_bits(data, bit_offset, bit_length);
    return raw_value;
}