#pragma once

#define BITS_PER_BYTE           ( 8 )
#define BIT(x)                  ( 1 << (x) )
#define MASK(x)                 ( BIT(x) - 1 )
#define SET_BIT(x, bit_flag)    ( x |= bit_flag )
#define UNSET_BIT(x, bit_flag)  ( x &= ~bit_flag )
#define IS_BIT_SET(x, bit_flag) ( (x & bit_flag) != 0 )
#define FLIP_BIT(x, bit_flag)   ( IS_BIT_SET(x, bit_flag) ? UNSET_BIT(x, bit_flag) : SET_BIT(x, bit_flag) )