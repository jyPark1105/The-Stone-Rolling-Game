#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include <glad/glad.h>

void printOpenGLInfo()
{
	// Print info
	//_putenv("CUDA_VISIBLE_DEVICES=0");

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* vendor = glGetString(GL_VENDOR);
	const GLubyte* version = glGetString(GL_VERSION);
	const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);

	printf("*********************************************************\n*\n");
	printf("*    GL Vendor            : %s\n", vendor);
	printf("*    GL Renderer          : %s\n", renderer);
	printf("*    GL Version Name      : %s\n", version);
	printf("*    GL Version           : %d.%d\n", major, minor);
	printf("*    GLSL Version         : %s\n", glslVersion);
	printf("*\n*********************************************************\n");
}

#endif