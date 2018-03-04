#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>


//-----------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const int maxLength, const char* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}


bool AreStringsEqualCaseSensitive(const char* a, const char* b)
{
	for(int index = 0; a[index] == b[index]; ++index){

		if(a[index] == '\0' && b[index] == '\0'){
			return true;
		}

		if(a[index] == '\0' || b[index] == '\0'){
			break;
		}
	}

	return false;
}

bool AreStringsEqualNonCaseSensitive(const char* a, const char* b)
{
	for(int index = 0; tolower(a[index]) == tolower(b[index]); ++index){

		if(a[index] == '\0' && b[index] == '\0'){
			return true;
		}

		if(a[index] == '\0' || b[index] == '\0'){
			break;
		}
	}

	return false;
}

void pretty_print_time(char* out_string, size_t buffer_size, double time_seconds)
{
    if(time_seconds < 0.001){
        // microseconds
        sprintf_s(out_string, buffer_size, "%.3f us", (time_seconds * 1000000.0));
    }else if(time_seconds < 1.0){
        // milliseconds
        sprintf_s(out_string, buffer_size, "%.3f ms", (time_seconds * 1000.0));
    }else{
        // seconds
        sprintf_s(out_string, buffer_size, "%.3f s", time_seconds);
    }
}

std::vector<std::string> tokenize_string_by_delimeter(const char* string, const char delimeter)
{
    std::vector<std::string> tokens;

    std::string token;
    const char* token_start = string;
    const char* token_end = strchr(string, (int)delimeter);

    while(nullptr != token_end){
        token = std::string(token_start, (size_t)(token_end - token_start));
        if(!token.empty()){
            tokens.push_back(token);
        }

        token_start = token_end + 1;
        token_end = strchr(token_start, (int)delimeter);
    }

    token = std::string(token_start);
    if(!token.empty()){
        tokens.push_back(token);
    }

    return tokens;
}