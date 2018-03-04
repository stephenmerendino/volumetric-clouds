#include "Engine/Core/Config.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/Console.hpp"

#include <vector>
#include <algorithm>
#include <ctime>

// -----------------------------------------
// Internal storage class
// -----------------------------------------

enum ConfigType
{
	CONFIG_TYPE_STRING,
	CONFIG_TYPE_BOOL,
	CONFIG_TYPE_INT,
	CONFIG_TYPE_FLOAT,
	CONFIG_TYPE_RGBA,
	NUM_CONFIG_TYPES
};

class ConfigSetting
{
public:
	char*		m_name;
	ConfigType	m_type;

	union
	{
		char*	m_stringValue;
		bool	m_boolValue;
		int		m_intValue;
		float	m_floatValue;
		Rgba	m_rgbaValue;
	};

public:
	ConfigSetting(const char* name, const char* value) 
		:m_name(nullptr)
		,m_stringValue(nullptr)
		,m_type(CONFIG_TYPE_STRING)
	{
		SetName(name);
		SetStringValue(value);
	};

	ConfigSetting(const char* name, bool value)
		:m_name(nullptr)
		,m_boolValue(value)
		,m_type(CONFIG_TYPE_BOOL)
	{
		SetName(name);
	}

	ConfigSetting(const char* name, int value) 
		:m_name(nullptr)
		,m_intValue(value)
		,m_type(CONFIG_TYPE_INT)
	{
		SetName(name);
	};

	ConfigSetting(const char* name, float value) 
		:m_name(nullptr)
		,m_floatValue(value)
		,m_type(CONFIG_TYPE_FLOAT)
	{
		SetName(name);
	};

	ConfigSetting(const char* name, const Rgba& rgba)
		:m_name(nullptr)
		,m_rgbaValue(rgba)
		,m_type(CONFIG_TYPE_RGBA)
	{
		SetName(name);
	}

	~ConfigSetting() {
		SAFE_DELETE_ARRAY(m_name);
		if(m_type == CONFIG_TYPE_STRING){
			SAFE_DELETE_ARRAY(m_stringValue);
		}
	};

	void SetName(const char* name)
	{
		SAFE_DELETE_ARRAY(m_name);
		CopyString(&m_name, name);
	}
	
	void SetStringValue(const char* newStringValue)
	{
		SAFE_DELETE_ARRAY(m_stringValue);
		CopyString(&m_stringValue, newStringValue);
	}


private:
	void CopyString(char** out_copy, const char* valueToCopy)
	{
		size_t stringLength = strlen(valueToCopy);
        *out_copy = new char[stringLength + 1];
		memcpy_s(*out_copy, stringLength, valueToCopy, stringLength);
		(*out_copy)[stringLength] = '\0';
	}

};

// -----------------------------------------
// File Loading and Processing
// -----------------------------------------

class RawConfigBuffer
{
public:
	char* m_rawBuffer;
	unsigned int m_size;
	unsigned int m_curIndex;

	static const char COMMENT_CHAR = '#';

public:
	RawConfigBuffer()
		:m_rawBuffer(nullptr)
		,m_size(0)
		,m_curIndex(0)
	{ }

	~RawConfigBuffer()
	{
		SAFE_DELETE_ARRAY(m_rawBuffer);
	}

	bool IsAtEnd() const { return m_curIndex >= m_size; }

	void GetToken(char** out_token)
	{
		AdvanceToNextToken();

		if(IsAtEnd()){
			return;
		}

		ParseNextToken(out_token);
	}

private:
	void AdvanceToNextToken()
	{
		// Keep skipping seperators and comments until we get to a good starting place or the end of file
		while(SkipSeperators() || SkipComments()){
			if(IsAtEnd()){
				break;
			}
		}
	}

	bool SkipSeperators()
	{
		if(IsCharTokenSeperator(m_rawBuffer[m_curIndex])){
			while(IsCharTokenSeperator(m_rawBuffer[m_curIndex])){
				++m_curIndex;
			}

			return true;
		}

		return false;
	}

	bool SkipComments()
	{
		// Advance past comments by ignoring the rest of the line
		if(m_rawBuffer[m_curIndex] == COMMENT_CHAR){
			while(m_rawBuffer[m_curIndex] != '\n' && m_rawBuffer[m_curIndex] != '\0'){
				++m_curIndex;
			}

			return true;
		}

		return false;
	}

	void ParseNextToken(char** out_token)
	{
		// If it is a string literal (eg "TestKey"), then ignore the quote and advance
		bool isStringLiteral = (m_rawBuffer[m_curIndex] == '"');
		if(isStringLiteral){
			++m_curIndex;
		}

		bool isColor = (m_rawBuffer[m_curIndex] == '<');

		// Find the ending index of the current token
		unsigned int endIndex = m_curIndex;
		while(endIndex < m_size){
			// If not a string literal, just stop at the next seperator char
			if(!isStringLiteral && !isColor && IsCharTokenSeperator(m_rawBuffer[endIndex])){
				break;
			}

			// If string literal, then advance til the ending quotation
			if(isStringLiteral && m_rawBuffer[endIndex] == '"'){
				break;
			}

			// Found the ending bracket for the color (eg <1,0,1,1>)
			if(isColor && m_rawBuffer[endIndex] == '>'){
				++endIndex; // Advance past the bracket
				break;
			}

			++endIndex;
		}

		// Copy out the current token into the out parameter
		unsigned int keySize = endIndex - m_curIndex;
        *out_token = new char[keySize + 1];
		memcpy_s(*out_token, keySize, &m_rawBuffer[m_curIndex], keySize);
		(*out_token)[keySize] = '\0';

		// Advance current index
		m_curIndex = endIndex;

		// String literal means we stopped on a quotation, we need to advance past it for the next pass
		if(isStringLiteral){
			++m_curIndex;
		}
	}

	bool IsCharTokenSeperator(char c)
	{
		return (c == ' ') || (c == '=') || (c == '\t') || (c == '\r') || (c == '\n');
	}
};

bool LoadConfigFileToRawBuffer(RawConfigBuffer* out_configFileBuffer, const char* filename)
{
	// Open up the file and make sure that worked
	FILE* file;
	errno_t errorCode = fopen_s(&file, filename, "rb");
	if(errorCode != 0){
		return false;
	}

	// Get the size of the file in bytes
	fseek(file, 0, SEEK_END);
	size_t filesize = ftell(file);
	fseek(file, 0, SEEK_SET);

	// Allocate memory to read the file into
	// I allocate one extra character so I can append a null termination char '\0' at the end
	(*out_configFileBuffer).m_size = filesize + 1;
	(*out_configFileBuffer).m_rawBuffer = new char[(*out_configFileBuffer).m_size];

	// Read in the file
	size_t bytesRead = fread_s((*out_configFileBuffer).m_rawBuffer, filesize, 1, filesize, file);
	ASSERT_OR_DIE(bytesRead == filesize, "Error, could not read all data from config file\n");

	// Make sure the data is null terminated
	(*out_configFileBuffer).m_rawBuffer[filesize] = '\0';

	// Close the file
	fclose(file);
	return true;
}

void LoadCommandLineToRawConfigBuffer(RawConfigBuffer* out_buffer, const char* commandLine)
{
	size_t stringLen = strlen(commandLine);

	*out_buffer = RawConfigBuffer();
	out_buffer->m_size = stringLen + 1;

	out_buffer->m_rawBuffer = new char[out_buffer->m_size];
	memcpy_s(out_buffer->m_rawBuffer, stringLen, commandLine, stringLen);
	out_buffer->m_rawBuffer[stringLen] = '\0';
}

ConfigType GetConfigTypeForRawValue(const char* rawValue)
{
	size_t stringLength = strlen(rawValue);

	// Check for true, TRUE, false, FALSE
	if(AreStringsEqualNonCaseSensitive(rawValue, "true") || AreStringsEqualNonCaseSensitive(rawValue, "false")){
		return CONFIG_TYPE_BOOL;
	}

	// Check if it matches the patterns for an Rgba value (eg 0xff2510ff or <1,0.5,2,1>)
	if(stringLength > 2){
		// Hex signature
		if(rawValue[0] == '0' && (rawValue[1] == 'x' || rawValue[1] == 'X')){
			return CONFIG_TYPE_RGBA;
		}

		// Opening brackets for bracket notation
		if(rawValue[0] == '<'){
			return CONFIG_TYPE_RGBA;
		}
	}

	// If it wasn't a bool, then lets check for a string next
	// If we find any non-digit or more than one decimal then it is automatically classified as a string
	int decimalCount = 0;
	for(int index = 0; rawValue[index] != '\0'; ++index){
		// Allow a minus at the beginning to specifiy negative numbers
		if(index == 0 && rawValue[0] == '-'){
			continue;
		}

		if(!isdigit(rawValue[index]) && rawValue[index] != '.'){
			return CONFIG_TYPE_STRING;
		}

		// Count up the decimals, and if more than one then we're calling it a string
		// Ain't no numbers be having more than one decimal, ya dig
		if(rawValue[index] == '.'){
			decimalCount++;
			if(decimalCount > 1){
				return CONFIG_TYPE_STRING;
			}
		}
	}

	// It has to be a number, so start off assuming int. If we find a decimal then its a float
	ConfigType valueType = CONFIG_TYPE_INT;
	for(int index = 0; rawValue[index] != '\0'; ++index){
		if(rawValue[index] == '.'){
			valueType = CONFIG_TYPE_FLOAT;
			break;
		}
	}

	return valueType;
}

bool IsRgbaSeperator(char c)
{
	return (c == ' ' || c == ',');
}

unsigned char GetNextRgbaValue_Bracket(unsigned int* out_curIndex, const char* rawValue)
{
	// Make sure we haven't hit the end bracket
	if(rawValue[*out_curIndex] == '>'){
		return 0;
	}

	// Advance to start of first token
	while(rawValue[*out_curIndex] == ' ' || rawValue[*out_curIndex] == ','){
		++(*out_curIndex);
	}

	// Read in and parse the character
	int value = 0;

	while(!IsRgbaSeperator(rawValue[*out_curIndex])){
		int currentNumberInRawValue = rawValue[*out_curIndex] - '0';

		value *= 10; // shift everything up by an order of magnitude
		value += currentNumberInRawValue; // Add in new "ones" digit

		++(*out_curIndex); // Advance to next character

		// Make sure we haven't hit the end bracket
		if(rawValue[*out_curIndex] == '>'){
			break;
		}
	}

	return (unsigned char)value;
}

unsigned char GetNextRgbaValue_Hex(unsigned int* out_curIndex, const char* rawValue)
{
	char highChar;
	char lowChar;

	if(rawValue[*out_curIndex] == '\0' || rawValue[*out_curIndex + 1] == '\0'){
		return 0;
	}

	highChar = (unsigned int)rawValue[*out_curIndex];
	(*out_curIndex)++;
	lowChar = (unsigned int)rawValue[*out_curIndex];
	(*out_curIndex)++;

	unsigned int highValue = 0;
	unsigned int lowValue = 0;

	if(isalpha(highChar)){
		highValue = (tolower(highChar) - 'a') + 10;
	} else{
		highValue = highChar - '0';
	}

	if(isalpha(lowChar)){
		lowValue = (tolower(lowChar) - 'a') + 10;
	} else{
		lowValue = lowChar - '0';
	}

	int packedValue = (highValue * 16) + lowValue;

	return (unsigned char)packedValue;
}

void ConvertRawValueToRgba(Rgba* out_rgba, const char* rawValue)
{
	// Check for bracket notation
	if(rawValue[0] == '<'){
		unsigned int curIndex = 1;

		unsigned char r = GetNextRgbaValue_Bracket(&curIndex, rawValue);
		unsigned char g = GetNextRgbaValue_Bracket(&curIndex, rawValue);
		unsigned char b = GetNextRgbaValue_Bracket(&curIndex, rawValue);
		unsigned char a = GetNextRgbaValue_Bracket(&curIndex, rawValue);

		*out_rgba = Rgba(r, g, b, a);
	} 
	// The only other option is that its hex
	else{
		unsigned int curIndex = 2; // Start after the 0x prefix

		unsigned char r = GetNextRgbaValue_Hex(&curIndex, rawValue);
		unsigned char g = GetNextRgbaValue_Hex(&curIndex, rawValue);
		unsigned char b = GetNextRgbaValue_Hex(&curIndex, rawValue);
		unsigned char a = GetNextRgbaValue_Hex(&curIndex, rawValue);

		*out_rgba = Rgba(r, g, b, a);
	}
}

void ConvertAndSetRawValue(const char* key, const char* rawValue)
{
	// Don't allow null strings for either keys or values
	if(key[0] == '\0' || rawValue == '\0'){
		return;
	}

	ConfigType valueType = GetConfigTypeForRawValue(rawValue);

	switch(valueType){
		case CONFIG_TYPE_STRING:
		{
			ConfigSet(key, rawValue);
		} break;

		case CONFIG_TYPE_BOOL:
		{
			bool realValue = AreStringsEqualNonCaseSensitive(rawValue, "true");
			ConfigSet(key, realValue);
		} break;

		case CONFIG_TYPE_INT:
		{
			int realValue = atoi(rawValue);
			ConfigSet(key, realValue);
		} break;

		case CONFIG_TYPE_FLOAT:
		{
			float realValue = (float)atof(rawValue);
			ConfigSet(key, realValue);
		} break;

		case CONFIG_TYPE_RGBA:
		{
			Rgba rgba;
			ConvertRawValueToRgba(&rgba, rawValue);
			ConfigSet(key, rgba);
		}break;
	}
}

bool IsKeyBooleanShortand(const char* key)
{
	return key[0] == '-' || key[0] == '+';
}

void ProcessAsBooleanShorthand(const char* key)
{
	if(key[0] != '-' && key[0] != '+'){
		return;
	}

	char flag = key[0];

	// False flag
	if(flag == '-'){
		ConfigSet(&key[1], false);
	}

	// True flag
	if(flag == '+'){
		ConfigSet(&key[1], true);
	}
}

void ProcessRawConfigBuffer(RawConfigBuffer& rawConfigBuffer)
{
	while(!rawConfigBuffer.IsAtEnd()){
		char* key = nullptr;
		char* rawValue = nullptr;

		rawConfigBuffer.GetToken(&key);

		if(!key){
			break;
		}

		if(IsKeyBooleanShortand(key)){
			ProcessAsBooleanShorthand(key);
			delete[] key;
			continue;
		}

		rawConfigBuffer.GetToken(&rawValue);

		if(!rawValue){
			delete[] key;
			break;
		}

		ConvertAndSetRawValue(key, rawValue);

		delete[] key;
		delete[] rawValue;
	}
}

// -----------------------------------------
// Internal static storage
// -----------------------------------------

static std::vector<ConfigSetting*> s_configSettings;
typedef std::vector<ConfigSetting*>::iterator ConfigIter;

// -----------------------------------------
// Internal Utility functions
// -----------------------------------------

ConfigSetting* FindConfigSetting(const char* name)
{
	ConfigIter iter = s_configSettings.begin();
	for(; iter != s_configSettings.end(); ++iter){
		ConfigSetting* setting = *iter;
		if(AreStringsEqualCaseSensitive(setting->m_name, name)){
			return setting;
		}
	}

	return nullptr;
}

void ConfigSet(const char* name, const void* value, ConfigType type)
{
	if(name[0] == '\0'){
		return;
	}

	ConfigSetting* setting = FindConfigSetting(name);

	if(!setting){
		switch(type){
			case CONFIG_TYPE_STRING:	s_configSettings.push_back(new ConfigSetting(name, (const char*)value));	break;
			case CONFIG_TYPE_BOOL:		s_configSettings.push_back(new ConfigSetting(name, *((bool*)value)));		break;
			case CONFIG_TYPE_INT:		s_configSettings.push_back(new ConfigSetting(name, *((int*)value)));		break;
			case CONFIG_TYPE_FLOAT:		s_configSettings.push_back(new ConfigSetting(name, *((float*)value)));		break;
			case CONFIG_TYPE_RGBA:		s_configSettings.push_back(new ConfigSetting(name, *((Rgba*)value)));		break;
		}
	} else{
		setting->m_type = type;
		switch(type){
			case CONFIG_TYPE_STRING:	setting->SetStringValue((const char*)value);		break;
			case CONFIG_TYPE_BOOL:		setting->m_boolValue	=	*((bool*)value);		break;
			case CONFIG_TYPE_INT:		setting->m_intValue		=	*((int*)value);			break;
			case CONFIG_TYPE_FLOAT:		setting->m_floatValue	=	*((float*)value);		break;
			case CONFIG_TYPE_RGBA:		setting->m_rgbaValue	=	*((Rgba*)value);		break;
		}
	}
}

bool ConfigGetValue(void* out_value, const char* name, ConfigType type)
{
	ConfigSetting* setting = FindConfigSetting(name);
	if(setting && setting->m_type == type){
		switch(type){
			case CONFIG_TYPE_STRING:	*((const char**)out_value)	= setting->m_stringValue;			break;
			case CONFIG_TYPE_BOOL:		*((bool*)out_value)			= setting->m_boolValue;				break;
			case CONFIG_TYPE_INT:		*((int*)out_value)			= setting->m_intValue;				break;
			case CONFIG_TYPE_FLOAT:		*((float*)out_value)		= setting->m_floatValue;			break;
			case CONFIG_TYPE_RGBA:		*((Rgba*)out_value)			= setting->m_rgbaValue;				break;
		}

		return true;
	}

	return false;
}

int SettingSortFunc(const void* a, const void* b)
{
	ConfigSetting* aSetting = *(ConfigSetting**)a;
	ConfigSetting* bSetting = *(ConfigSetting**)b;

	// Compare their types
	if(aSetting->m_type < bSetting->m_type){
		return -1;
	} else if(aSetting->m_type > bSetting->m_type){
		return 1;
	} else{
		return strcmp(aSetting->m_name, bSetting->m_name);
	}
}


void SortConfigSettings()
{
	std::qsort(s_configSettings.data(), s_configSettings.size(), sizeof(ConfigSetting*), SettingSortFunc);
}

// -----------------------------------------
// PUBLIC API
// -----------------------------------------

// -----------------------------------------
// Startup and Shutdown
// -----------------------------------------

bool ConfigSystemStartup(char const* filename, const char* commandLine)
{
	// Load the config file first
	bool success = true;
	success = ConfigLoadFile(filename);

	// Load the command line second so that they can override file settings
	RawConfigBuffer commandLineBuffer;
	LoadCommandLineToRawConfigBuffer(&commandLineBuffer, commandLine);
	ProcessRawConfigBuffer(commandLineBuffer);

	return success;
}

void ConfigSystemShutdown()
{
	ConfigIter iter = s_configSettings.begin();
	for(; iter != s_configSettings.end(); ++iter){
		SAFE_DELETE(*iter);
	}
    s_configSettings.clear();
}

// -----------------------------------------
// Gettings & Setters 
// -----------------------------------------

void ConfigSet(const char* name, const char* value)
{
	if(value[0] == '\0'){
		return;
	}

	ConfigSet(name, value, CONFIG_TYPE_STRING);
}

void ConfigSet(const char* name, const bool value)
{
	ConfigSet(name, (void*)&value, CONFIG_TYPE_BOOL);
}

void ConfigSet(const char* name, const int value)
{
	ConfigSet(name, (void*)&value, CONFIG_TYPE_INT);
}

void ConfigSet(const char* name, const float value)
{
	ConfigSet(name, (void*)&value, CONFIG_TYPE_FLOAT);
}

void ConfigSet(const char* name, const Rgba& rgba)
{
	ConfigSet(name, (void*)&rgba, CONFIG_TYPE_RGBA);
}

bool ConfigGetString(char** out_value, const char* name)
{
	return ConfigGetValue(out_value, name, CONFIG_TYPE_STRING);
}

bool ConfigGetBool(bool* out_value, const char* name)
{
	return ConfigGetValue(out_value, name, CONFIG_TYPE_BOOL);
}

bool ConfigGetInt(int* out_value, const char* name)
{
	return ConfigGetValue(out_value, name, CONFIG_TYPE_INT);
}

bool ConfigGetFloat(float* out_value, const char* name)
{
	return ConfigGetValue(out_value, name, CONFIG_TYPE_FLOAT);
}

bool ConfigGetRgba(Rgba* out_value, const char* name)
{
	return ConfigGetValue(out_value, name, CONFIG_TYPE_RGBA);
}

bool IsConfigSet(const char* name)
{
	return FindConfigSetting(name) != nullptr;
}

void ConfigUnset(const char* name)
{
	if(IsConfigSet(name)){
		ConfigIter iter = s_configSettings.begin();
		for(; iter != s_configSettings.end(); ++iter){
			ConfigSetting* setting = *iter;
			if(setting->m_name == name){
				s_configSettings.erase(iter);
				return;
			}
		}
	}
}

bool ConfigLoadFile(const char* filename)
{
	RawConfigBuffer configFileBuffer;
	if(!LoadConfigFileToRawBuffer(&configFileBuffer, filename)){
		return false;
	}

	ProcessRawConfigBuffer(configFileBuffer);

	return true;
}

// -----------------------------------------
// Debugging
// -----------------------------------------

void ConfigList()
{
	SortConfigSettings();
	DebuggerPrintf("Current Config\n");
	ConfigIter iter = s_configSettings.begin();
	for(; iter != s_configSettings.end(); ++iter){
		ConfigSetting* setting = *iter;
		switch(setting->m_type)
		{
			case CONFIG_TYPE_STRING:	DebuggerPrintf("%s = %s (string)\n", setting->m_name, setting->m_stringValue);					break;
			case CONFIG_TYPE_BOOL:		DebuggerPrintf("%s = %s (bool)\n", setting->m_name, setting->m_boolValue ? "true" : "false");	break;
			case CONFIG_TYPE_INT:		DebuggerPrintf("%s = %i (int)\n", setting->m_name, setting->m_intValue);						break;
			case CONFIG_TYPE_FLOAT:		DebuggerPrintf("%s = %f (float)\n", setting->m_name, setting->m_floatValue);					break;
			case CONFIG_TYPE_RGBA:		DebuggerPrintf("%s = <%u, %u, %u, %u> (rgba)\n", setting->m_name, setting->m_rgbaValue.r,
																										  setting->m_rgbaValue.g,
																										  setting->m_rgbaValue.b,
																										  setting->m_rgbaValue.a);		break;
		}
	}
}

// -----------------------------------------
// File I/O
// -----------------------------------------

// Interal helper function for printing type headings
void PrintTypeHeading(FILE* file, ConfigType type)
{
	switch (type)
	{
		case CONFIG_TYPE_STRING:	fprintf(file, "# [STRINGS]\n");		break;
		case CONFIG_TYPE_BOOL:		fprintf(file, "# [BOOLS]\n");		break;
		case CONFIG_TYPE_INT:		fprintf(file, "# [INTS]\n");		break;
		case CONFIG_TYPE_FLOAT:		fprintf(file, "# [FLOATS]\n");		break;
		case CONFIG_TYPE_RGBA:		fprintf(file, "# [COLORS]\n");		break;
	}
}

bool ConfigSystemSaveToFile(const char* filename)
{
	// Open output file
	FILE* file;
	errno_t errorCode = fopen_s(&file, filename, "w");
	if(errorCode != 0){
		return false;
	}

	// Build date/time string
	std::time_t rawTime = std::time(nullptr);
	tm localTime;
	localtime_s(&localTime, &rawTime);
	char timeString[100];
	strftime(timeString, 100, "# Saved on %D @ %r\0", &localTime);

	// Print the file heading
	fprintf(file, "# Config Settings\n%s\n\n\n", timeString);

	if(s_configSettings.size() == 0){
		fprintf(file, "# No Config Settings Found");
		fclose(file);
		return true;
	}
    
	// Sort and get the first config type
	SortConfigSettings();
	ConfigType currentType = s_configSettings[0]->m_type;
	PrintTypeHeading(file, currentType);

	// Loop through each setting and print it to the file
	ConfigIter iter = s_configSettings.begin();
	for(; iter != s_configSettings.end(); ++iter){
		ConfigSetting* setting = *iter;
		
		// If the type changes then print a nice heading
		if(currentType != setting->m_type){
			fprintf(file, "\n\n");
			currentType = setting->m_type;
			PrintTypeHeading(file, currentType);
		}

		switch(setting->m_type)
		{
			case CONFIG_TYPE_STRING:	fprintf(file, "\"%s\" = \"%s\"\n", setting->m_name, setting->m_stringValue);					break;
			case CONFIG_TYPE_BOOL:		fprintf(file, "%s = %s\n", setting->m_name, setting->m_boolValue ? "true" : "false");		break;
			case CONFIG_TYPE_INT:		fprintf(file, "%s = %i\n", setting->m_name, setting->m_intValue);							break;
			case CONFIG_TYPE_FLOAT:		fprintf(file, "%s = %f\n", setting->m_name, setting->m_floatValue);							break;
			case CONFIG_TYPE_RGBA:		fprintf(file, "%s = <%u, %u, %u, %u>\n", setting->m_name, setting->m_rgbaValue.r,
																										  setting->m_rgbaValue.g,
																										  setting->m_rgbaValue.b,
																										  setting->m_rgbaValue.a);		break;
		}
	}

	// Close the file
	fclose(file);
	return true;
}

COMMAND(list_config, "list all loaded config options")
{
	SortConfigSettings();
	ConfigIter iter = s_configSettings.begin();
	for(; iter != s_configSettings.end(); ++iter){
		ConfigSetting* setting = *iter;
		switch(setting->m_type)
		{
		case CONFIG_TYPE_STRING:	console_info("%s = \"%s\" (string)", setting->m_name, setting->m_stringValue);					break;
		case CONFIG_TYPE_BOOL:		console_info("%s = %s (bool)", setting->m_name, setting->m_boolValue ? "true" : "false");		break;
		case CONFIG_TYPE_INT:		console_info("%s = %i (int)", setting->m_name, setting->m_intValue);							break;
		case CONFIG_TYPE_FLOAT:		console_info("%s = %f (float)", setting->m_name, setting->m_floatValue);						break;
		case CONFIG_TYPE_RGBA:		console_info("%s = <%u, %u, %u, %u> (rgba)", setting->m_name, setting->m_rgbaValue.r,
																										  setting->m_rgbaValue.g,
																										  setting->m_rgbaValue.b,
																										  setting->m_rgbaValue.a);	break;
		}
	}
}