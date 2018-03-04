#include "Engine/Core/Common.hpp"
#include <stdio.h>

char* bytes_to_string(char* out_bytes_string, size_t string_len, size_t bytes)
{
    if(bytes < KiB(1)){
        sprintf_s(out_bytes_string, string_len, "%i B", bytes);
    }else if(bytes < MiB(1)){
        sprintf_s(out_bytes_string, string_len, "%.3f KiB", (float)bytes / KiB(1));
    }else if(bytes < GiB(1)){
        sprintf_s(out_bytes_string, string_len, "%.3f MiB", (float)bytes / MiB(1));
    }else{
        sprintf_s(out_bytes_string, string_len, "%.3f GiB", (float)bytes / GiB(1));
    }

    return out_bytes_string;
}