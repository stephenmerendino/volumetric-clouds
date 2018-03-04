#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <stdlib.h>

bool LoadBinaryFileToBuffer(char** out_buffer, const char* filePath)
{
	// Open up the file and make sure that worked
	FILE* file;
	errno_t errorCode = fopen_s(&file, filePath, "rb");
	if(errorCode != 0){
		return false;
	}

	// Get the size of the file in bytes
	fseek(file, 0, SEEK_END);
	size_t filesize = ftell(file);
	fseek(file, 0, SEEK_SET);

	// Allocate memory to read the file into
	// I allocate one extra character so I can append a null termination char '\0' at the end
	*out_buffer = new char[filesize + 1];

	// Read in the file
	size_t bytesRead = fread_s(*out_buffer, filesize, 1, filesize, file);
	ASSERT_OR_DIE(bytesRead == filesize, "Error, could not read all data from config file\n");

	// Make sure the data is null terminated
	(*out_buffer)[filesize] = '\0';

	// Close the file
	fclose(file);
	return true;
}

bool SaveBufferToBinaryFile(const char* buffer, const char* filePath)
{
	// Open output file
	FILE* file;
	errno_t errorCode = fopen_s(&file, filePath, "wb");
	if(errorCode != 0){
		return false;
	}

	fprintf(file, "%s", buffer);

	fclose(file);
	return true;
}

bool LoadBinaryFileToBuffer(const std::string& filePath, std::vector<unsigned char>& out_buffer){
	FILE* file;
	errno_t errorCode = fopen_s(&file, filePath.c_str(), "rb");
	if(errorCode != 0){
		return false;
	}

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	out_buffer.resize(size);

	size_t bytesRead = fread(out_buffer.data(), 1, size, file);

	std::string errorMessage = Stringf("Error, could not read all data from file %s\n", filePath.c_str());
	ASSERT_OR_DIE(bytesRead == size, errorMessage.c_str());

	fclose(file);
	return true;
}

bool SaveBufferToBinaryFile(const std::string& filePath, const std::vector<unsigned char>& buffer){
	FILE* file;
	errno_t errorCode = fopen_s(&file, filePath.c_str(), "wb");
	if(errorCode != 0){
		return false;
	}

	size_t bytesWritten = fwrite(buffer.data(), 1, buffer.size(), file);

	std::string errorMessage = Stringf("Error, could not write all data to file %s\n", filePath.c_str());
	ASSERT_OR_DIE(bytesWritten == buffer.size(), errorMessage.c_str());

	fclose(file);
	return true;
}

bool LoadTextFileLinesToBuffer(const std::string& filePath, std::vector<std::string>& out_buffer)
{
	FILE* file;
	errno_t errorCode = fopen_s(&file, filePath.c_str(), "r");
	if(errorCode != 0){
		return false;
	}

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

    char* buffer = new char[size + 1];
	size_t bytesRead = fread(buffer, 1, size, file);
    buffer[bytesRead] = '\0';
    fclose(file);

    out_buffer = tokenize_string_by_delimeter(buffer, '\n');

    return true;
}

bool SaveTextFileLinesToBuffer(const std::string& filePath, const std::vector<std::string>& buffer)
{
	FILE* file;
	errno_t errorCode = fopen_s(&file, filePath.c_str(), "w");
	if(errorCode != 0){
		return false;
	}

    for(const std::string& line : buffer){
    	fwrite(line.c_str(), 1, line.size(), file);
        fwrite("\n", 1, 1, file);
    }

    fclose(file);

    return true;
}