#pragma once

#include <stdint.h>

typedef uint8_t         u8;
typedef uint16_t        u16;
typedef uint32_t        u32;
typedef uint64_t        u64;

typedef int8_t          i8;
typedef int16_t         i16;
typedef int32_t         i32;
typedef int64_t         i64;

typedef float           f32;
typedef double          f64;

typedef unsigned char   byte;
typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;

enum class Err : u8
{
    OK,
    FAIL,
    NUM_ERR_TYPES
};

#define internal_function static
#define local_persist static
#define global_variable static