#include "Engine/Core/process.hpp"
#include "Engine/Core/Common.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

std::string get_app_name()
{
    constexpr int MAX_NAME_LENGTH = 256;
    TCHAR t_char_name[MAX_NAME_LENGTH];
    DWORD name_length = GetModuleFileName(NULL, (LPTSTR)&t_char_name, MAX_NAME_LENGTH);
    if(0 == name_length){
        return "";
    }

    char c_name[MAX_NAME_LENGTH];

    size_t num_chars_converted;
    wcstombs_s(&num_chars_converted, c_name, MAX_NAME_LENGTH, t_char_name, MAX_NAME_LENGTH);

    return std::string(c_name);
}

bool launch_program(const char* filename, const char* arguments)
{
    STARTUPINFOA startup_info;
    MemZero(&startup_info);
    startup_info.cb = sizeof(STARTUPINFOA);

    // don't start active
    startup_info.dwFlags = STARTF_USESHOWWINDOW;
    startup_info.wShowWindow = SW_SHOWNOACTIVATE;

    PROCESS_INFORMATION process_info;
    MemZero(&process_info);

    char command_and_args[1024];
    sprintf_s(command_and_args, "\"%s\" %s", filename, arguments);

    BOOL success = ::CreateProcessA(NULL, command_and_args, NULL, NULL, FALSE, 0, NULL, NULL, &startup_info, &process_info);
    return (success == TRUE);
}