#include "Engine/Core/FileBinaryStream.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

FileBinaryStream::FileBinaryStream()
	:m_file(nullptr)
{
}

FileBinaryStream::~FileBinaryStream()
{
	close();
}

bool FileBinaryStream::is_open()
{
	return m_file != nullptr;
}

bool FileBinaryStream::open_for_write(const char* filename)
{
	ASSERT_OR_DIE(!is_open(), Stringf("Error: Cannot open file[%s] that is already open", filename));
	m_filename = filename;
	fopen_s(&m_file, m_filename, "wb");
	ASSERT_OR_DIE(is_open(), Stringf("Error: Could not open file [%s] for write", filename));
	return is_open();
}

bool FileBinaryStream::open_for_read(const char* filename)
{
	ASSERT_OR_DIE(!is_open(), Stringf("Error: Cannot open file[%s] that is already open", filename));
	m_filename = filename;
	fopen_s(&m_file, m_filename, "rb");
	ASSERT_OR_DIE(is_open(), Stringf("Error: Could not open file [%s] for read", filename));
	return is_open();
}

void FileBinaryStream::close()
{
	if(is_open()){
		fclose(m_file);
		m_file = nullptr;
		m_filename = nullptr;
	}
}

unsigned int FileBinaryStream::write_bytes(void* bytes, unsigned int count)
{
	ASSERT_OR_DIE(is_open(), Stringf("Error: Cannot write bytes to file[%s] because it was never opened", m_filename));
	return (unsigned int)fwrite(bytes, 1, count, m_file);
}

unsigned int FileBinaryStream::read_bytes(void* out_bytes, unsigned int count)
{
	ASSERT_OR_DIE(is_open(), Stringf("Error: Cannot read bytes from file[%s] because it was never opened", m_filename));
	return (unsigned int)fread_s(out_bytes, count, 1, count, m_file);
}