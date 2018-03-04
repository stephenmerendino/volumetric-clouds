#pragma once

#include <string>
#include <vector>

bool LoadBinaryFileToBuffer(char** out_buffer, const char* filePath);
bool SaveBufferToBinaryFile(const char* buffer, const char* filePath);

bool LoadBinaryFileToBuffer(const std::string& filePath, std::vector<unsigned char>& out_buffer);
bool SaveBufferToBinaryFile(const std::string& filePath, const std::vector<unsigned char>& buffer);

bool LoadTextFileLinesToBuffer(const std::string& filePath, std::vector<std::string>& out_buffer);
bool SaveTextFileLinesToBuffer(const std::string& filePath, const std::vector<std::string>& buffer);