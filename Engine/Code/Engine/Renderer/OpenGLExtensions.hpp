#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>

#include "ThirdParty/gl/glext.h"

namespace OpenGL{
	void InitExtensions();
}

extern PFNGLGENBUFFERSPROC		glGenBuffers;
extern PFNGLDELETEBUFFERSPROC	glDeleteBuffers;
extern PFNGLBINDBUFFERPROC		glBindBuffer;
extern PFNGLBUFFERDATAPROC		glBufferData;
extern PFNGLGENERATEMIPMAPPROC	glGenerateMipmap;