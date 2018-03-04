#include "Engine/Renderer/OpenGLExtensions.hpp"

PFNGLGENBUFFERSPROC			glGenBuffers		= nullptr;
PFNGLDELETEBUFFERSPROC		glDeleteBuffers		= nullptr;
PFNGLBINDBUFFERPROC			glBindBuffer		= nullptr;
PFNGLBUFFERDATAPROC			glBufferData		= nullptr;
PFNGLGENERATEMIPMAPPROC		glGenerateMipmap	= nullptr;

void OpenGL::InitExtensions(){
	glGenBuffers				= (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
	glDeleteBuffers				= (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
	glBindBuffer				= (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
	glBufferData				= (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
	glGenerateMipmap			= (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
}