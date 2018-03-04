#pragma once

#include "Engine/Core/BinaryStream.hpp"
#include "Engine/Core/bit.h"
#include "Engine/Core/types.h"

class BitPacker : public BinaryStream
{
    public:
        size_t m_buffer_size;
        void* m_buffer;

        u32 m_bits_written;
        u32 m_bits_read;

    public:
        BitPacker(size_t buffer_size);
        virtual ~BitPacker();

        virtual u32 write_bytes(void* bytes, size_t byte_count) override;
        virtual u32 read_bytes(void* out_bytes, size_t byte_count) override;

    public:
        u32 write_bits(void* bytes, size_t bit_count);
        void write_byte(byte b, size_t bit_count);
        void write_bit(bool bit_is_set);

        u32 read_bits(void* out_bytes, size_t out_bytes_size, size_t bit_count);
        byte read_byte(size_t bit_count);
        bool read_bit();

        void write_compressed_float(f32 value, f32 min, f32 max, size_t bit_count);
        f32 read_compressed_float(f32 min, f32 max, size_t bit_count);

        byte* get_current_write_byte();
        byte* get_current_read_byte();

        u32 get_num_bytes_written();
        u32 get_num_bytes_read();

    public:
        template<typename T>
        void write(const T& value, size_t bit_count)
        {
            byte* buffer = (byte*)&value;

            // if should flip, then flip the bytes around
            if(should_flip()){
                byte* flipped_buffer = (byte*)_alloca(sizeof(T));
                ares::flip_bytes_to_buffer(buffer, flipped_buffer, sizeof(T));
                buffer = flipped_buffer;
            }

            write_bits((void*)buffer, bit_count);
        }

        template<typename T>
        T read(size_t bit_count)
        {
            T val = 0;
            byte* buffer = (byte*)&val;

            // read in up to bit_count
            read_bits(buffer, sizeof(T), bit_count);

            // if signed and value is negative then pad out 1's for two's complement
            if(std::is_signed<T>()){
                u32 num_bytes = (u32)bit_count / BITS_PER_BYTE;
                u32 num_remaining_bits = (u32)bit_count % BITS_PER_BYTE;

                // get most significant byte
                size_t msb_idx = (num_bytes - 1) + (num_remaining_bits > 0 ? 1 : 0);
                byte* msb = buffer + msb_idx;

                // check sign bit 
                byte msb_mask = BIT(num_remaining_bits - 1);
                bool is_sign_bit_set = ((*msb & msb_mask) != 0);

                // if sign bit is set then pad out 1's for rest of value
                if(is_sign_bit_set){
                    // pad remaining bits
                    byte pad_mask = ~MASK(num_remaining_bits);
                    *msb |= pad_mask; 

                    // pad remaining bytes
                    for(size_t byte_idx = msb_idx + 1; byte_idx < sizeof(T); ++byte_idx){
                        buffer[byte_idx] = 0xFF;
                    }
                }
            }

            // flips bytes if host order is big endian 
            if(should_flip()){
                ares::flip_bytes_in_place(buffer, sizeof(T));
            }

            return val;
        }
};