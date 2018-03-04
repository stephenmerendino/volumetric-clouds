#pragma once

#include "Engine/Core/Common.hpp"
#include <stdio.h>
#include <string>

class BinaryStream
{
public:
	Endianness m_stream_order;

public:
    BinaryStream();
    BinaryStream(Endianness stream_order);
	virtual ~BinaryStream(){};

	virtual void write_byte(byte_t byte);
	virtual void read_byte(byte_t& out_byte);

	virtual unsigned int write_bytes(void* bytes, unsigned int count) = 0;
	virtual unsigned int read_bytes(void* out_bytes, unsigned int count) = 0;

	unsigned int write_endian_aware_bytes(void* bytes, unsigned int count);
	unsigned int read_endian_aware_bytes(void* out_bytes, unsigned int count);

	inline bool should_flip() { return m_stream_order != GetHostOrder(); }

	template<typename T>
	bool write(const T& v)
	{
		return write_endian_aware_bytes((void*)&v, sizeof(v)) == sizeof(v);
	}

	template<typename T>
	bool read(T& t)
	{
		return read_endian_aware_bytes((void*)&t, sizeof(t)) == sizeof(t);
	}

	bool read(void* out_bytes, unsigned int count)
	{
		return read_endian_aware_bytes(out_bytes, count) == count;
	}
};

template<>
inline
bool BinaryStream::write(const std::string& str)
{
	bool wrote_all = (write_endian_aware_bytes((void*)str.c_str(), (unsigned int)str.size()) == (unsigned int)str.size());
	write_byte(NULL);
	return wrote_all;
}