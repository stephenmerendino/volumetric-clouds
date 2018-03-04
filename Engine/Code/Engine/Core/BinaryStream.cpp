#include "Engine/Core/BinaryStream.hpp"

BinaryStream::BinaryStream()
{
}

BinaryStream::BinaryStream(Endianness stream_order)
    :m_stream_order(stream_order)
{
}

void BinaryStream::write_byte(byte_t byte)
{
	write_bytes(&byte, 1);
}

void BinaryStream::read_byte(byte_t& out_byte)
{
	read_bytes(&out_byte, 1);
}

unsigned int BinaryStream::write_endian_aware_bytes(void* bytes, unsigned int count)
{
	if(should_flip()){
		byte_t* temp = (byte_t*)_alloca(count);
		CopyFlippedBytes(temp, bytes, count);
		return write_bytes(temp, count);
	}
	else{
		return write_bytes(bytes, count);
	}
}

unsigned int BinaryStream::read_endian_aware_bytes(void* out_bytes, unsigned int count)
{
	if(should_flip()){
		int bytes_read = read_bytes(out_bytes, count);
		FlipBytesInPlace(out_bytes, count);
		return bytes_read;
	}
	else{
		return read_bytes(out_bytes, count);
	}
}