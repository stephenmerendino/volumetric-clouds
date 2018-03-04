#pragma once

#include <stdint.h>
#include <memory.h>

//----------------------------------------------------------
// Types
//

typedef unsigned char byte_t;

//----------------------------------------------------------
// Function Helpers
//

#define UNUSED(param) (void)(param)
#define SAFE_DELETE(ptr) { if(nullptr != ptr){ delete ptr; ptr = nullptr; } }
#define SAFE_DELETE_ARRAY(ptr) { if(nullptr != ptr){ delete[] ptr; ptr = nullptr; } }


//----------------------------------------------------------
// Memory
//

template<typename T>
void MemZero(T* t)
{
	memset(t, 0, sizeof(T));
}

template<typename T>
void MemZeroArray(T* t, int size)
{
	memset(t, 0, sizeof(T) * size);
}

template<typename T>
void Swap(T* a, T* b)
{
	T temp = *a;
	*a = *b;
	*b = temp;
}


//----------------------------------------------------------
// Endianness
//

static constexpr uint32_t ENDIAN_CHECK = 0x01020304; 
bool constexpr IsLittleEndian() 
{
   return ((byte_t*)(&ENDIAN_CHECK))[0] == 0x04; 
}

enum Endianness
{
   LITTLE_ENDIAN, 
   BIG_ENDIAN, 
};

Endianness constexpr GetHostOrder()
{
	return IsLittleEndian() ? LITTLE_ENDIAN : BIG_ENDIAN;
}

inline
void FlipBytesInPlace(void* buffer, unsigned int count)
{
	byte_t* byte_array = (byte_t*)buffer;

	unsigned int split = count / 2;
	for(unsigned int byte_index = 0; byte_index < split; ++byte_index){
		byte_t* a = &byte_array[byte_index];
		byte_t* b = &byte_array[count - (byte_index + 1)];
		Swap(a, b);
	}
}

inline 
void CopyFlippedBytes(void* flip_buffer, const void* original_buffer, unsigned int count)
{
	memcpy(flip_buffer, original_buffer, count);
	FlipBytesInPlace(flip_buffer, count);
}

#define KiB(x) ((float)x * 1024.0f)
#define MiB(x) ((float)x * 1024.0f * 1024.0f)
#define GiB(x) ((float)x * 1024.0f * 1024.0f * 1024.0f)

char* bytes_to_string(char* out_bytes_string, size_t string_len, size_t bytes);