#pragma once

#include "Engine/Core/BinaryStream.hpp"
#include <stdio.h>

class FileBinaryStream : public BinaryStream
{
public:
	const char* m_filename;
	FILE* m_file;

public:
	FileBinaryStream();
	~FileBinaryStream();

	bool is_open();
	bool open_for_write(const char* filename);
	bool open_for_read(const char* filename);
	void close();

	virtual unsigned int write_bytes(void* bytes, unsigned int count) override;
	virtual unsigned int read_bytes(void* out_bytes, unsigned int count) override;
};