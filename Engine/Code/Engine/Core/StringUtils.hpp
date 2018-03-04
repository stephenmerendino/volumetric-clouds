#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>

#define COMBINE1(x,y) x##y
#define COMBINE(x,y) COMBINE1(x,y)

//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );

bool AreStringsEqualNonCaseSensitive(const char* a, const char* b);
bool AreStringsEqualCaseSensitive(const char* a, const char* b);

void pretty_print_time(char* out_string, size_t buffer_size, double time_seconds);

std::vector<std::string> tokenize_string_by_delimeter(const char* string, const char delimeter);