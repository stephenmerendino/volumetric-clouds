#include "Engine/Core/bit_packer.h"
#include "Engine/Core/Common.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

BitPacker::BitPacker(const size_t buffer_size)
    :BinaryStream(LITTLE_ENDIAN)
    ,m_buffer_size(buffer_size)
    ,m_bits_written(0)
    ,m_bits_read(0)
{
    m_buffer = malloc(m_buffer_size);
    memset(m_buffer, 0, m_buffer_size);
}

BitPacker::~BitPacker()
{
    free(m_buffer);
}

u32 BitPacker::write_bytes(void* bytes, size_t count)
{
    ASSERT_OR_DIE(nullptr != bytes, "bytes is null");
    ASSERT_OR_DIE(count <= m_buffer_size - get_num_bytes_written(), "Not enough space left to write");
    return write_bits(bytes, count * BITS_PER_BYTE);
}

u32 BitPacker::read_bytes(void* out_bytes, size_t count)
{
    ASSERT_OR_DIE(nullptr != out_bytes, "out_bytes is null");
    ASSERT_OR_DIE(count <= m_buffer_size - get_num_bytes_written(), "Not enough space left to read");
    return read_bits(out_bytes, count, count * BITS_PER_BYTE);
}

u32 BitPacker::write_bits(void* bytes, size_t bit_count)
{
    ASSERT_OR_DIE(nullptr != bytes, "bytes is null");

    size_t num_bytes = bit_count / BITS_PER_BYTE;
    ASSERT_OR_DIE(num_bytes <= m_buffer_size - get_num_bytes_written(), "Not enough space left to write");
    size_t num_remaining_bits = bit_count % BITS_PER_BYTE;
    size_t start_bits_written = m_bits_written;

    byte* buffer = (byte*)bytes;

    // write out whole bytes
    for(size_t byte_idx = 0; byte_idx < num_bytes; ++byte_idx){
        write_byte(buffer[byte_idx], BITS_PER_BYTE);
    }

    // write out the remaining bits
    if(num_remaining_bits > 0){
        write_byte(buffer[num_bytes], num_remaining_bits);
    }

    return (u32)(m_bits_written - start_bits_written);
}

void BitPacker::write_byte(byte b, size_t bit_count)
{
    for(size_t bit_idx = 0; bit_idx < bit_count; ++bit_idx){
        bool is_bit_set = IS_BIT_SET(b, BIT(bit_idx));
        write_bit(is_bit_set);
    }
}

void BitPacker::write_bit(bool bit_is_set)
{
    byte* current_byte = get_current_write_byte();
    size_t bit_offset = m_bits_written % BITS_PER_BYTE;

    byte mask = BIT(bit_offset);
    byte offmask = ~mask;

    // turn the bit off
    (*current_byte) = (*current_byte) & offmask;

    // turn it back on if needed
    if(bit_is_set){
        (*current_byte) |= mask;
    }
     
    ++m_bits_written;
}

u32 BitPacker::read_bits(void* out_bytes, size_t out_bytes_size, size_t bit_count)
{
    ASSERT_OR_DIE(nullptr != out_bytes, "out_bytes is null");

    memset(out_bytes, 0, out_bytes_size);

    size_t num_bytes = bit_count / BITS_PER_BYTE;
    ASSERT_OR_DIE(num_bytes <= m_buffer_size - get_num_bytes_read(), "Not enough space left to read");
    size_t num_remaining_bits = bit_count % BITS_PER_BYTE; 
    size_t start_bits_read = m_bits_read;

    byte* buffer = (byte*)out_bytes;

    for(size_t byte_idx = 0; byte_idx < num_bytes; ++byte_idx){
        byte b = read_byte(8);
        buffer[byte_idx] = b;
    }

    if(num_remaining_bits > 0){
        byte b = read_byte(num_remaining_bits);
        buffer[num_bytes] = b;
    }

    return (u32)(m_bits_read - start_bits_read);
}

byte BitPacker::read_byte(size_t bit_count)
{
    byte b = 0;
    for(size_t bit_idx = 0; bit_idx < bit_count; ++bit_idx){
        bool is_bit_set = read_bit();
        if(is_bit_set){
            byte mask = BIT(bit_idx);
            b |= mask;
        }
    }
    return b;
}

bool BitPacker::read_bit()
{
    byte* b = get_current_read_byte(); 

    size_t bit_offset = m_bits_read % BITS_PER_BYTE;
    byte mask = BIT(bit_offset);
    bool is_bit_set = ((*b) & mask) != 0;

    ++m_bits_read;

    return is_bit_set;
}

void BitPacker::write_compressed_float(f32 value, f32 min, f32 max, size_t bit_count)
{
    float normalized = MapFloatToRange(value, min, max, 0.0f, 1.0f);
    u32 max_fidelity = MASK(bit_count);
    u32 compressed_value = (u32)((f32)max_fidelity * normalized);
    write_bits(&compressed_value, bit_count);
}

f32 BitPacker::read_compressed_float(f32 min, f32 max, size_t bit_count)
{
    f32 max_fidelity = (f32)MASK(bit_count);
    u32 compressed_value;
    read_bits(&compressed_value, sizeof(u32), bit_count);
    return MapFloatToRange((f32)compressed_value, 0.0f, max_fidelity, min, max);
}

byte* BitPacker::get_current_write_byte()
{
    size_t current_byte = m_bits_written / BITS_PER_BYTE;
    return (byte*)m_buffer + current_byte;
}

byte* BitPacker::get_current_read_byte()
{
    size_t current_byte = m_bits_read / BITS_PER_BYTE;
    return (byte*)m_buffer + current_byte;
}

u32 BitPacker::get_num_bytes_written()
{
    return ((m_bits_written + 7) / BITS_PER_BYTE);
}

u32 BitPacker::get_num_bytes_read()
{
    return ((m_bits_read + 7) / BITS_PER_BYTE);
}