#pragma once

#define MAX_FRAMES_PER_CALLSTACK 128
#define MAX_SYMBOL_NAME_LENGTH 1024
#define MAX_FILENAME_LENGTH 1024
#define MAX_DEPTH 128

struct callstack_line_t 
{
   char filename[MAX_FILENAME_LENGTH];
   char function_name[MAX_SYMBOL_NAME_LENGTH];
   unsigned int line;
   unsigned int offset;
};

class Callstack
{
public:
    unsigned int hash;
    unsigned int frame_count;
    void* frames[MAX_FRAMES_PER_CALLSTACK];

public:
    Callstack(); 
};

bool            callstack_system_init();
void            callstack_system_shutdown();

Callstack*      create_callstack(unsigned int skip_frames);
void            destroy_callstack(Callstack *c);

unsigned int    callstack_get_lines(callstack_line_t *line_buffer, unsigned int const max_lines, Callstack *cs);