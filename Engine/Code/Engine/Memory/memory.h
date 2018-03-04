#pragma once

#include <new.h>

void* operator new(const size_t size, void* p);
void operator delete(void* s, void* p);