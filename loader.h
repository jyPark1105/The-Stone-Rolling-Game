#ifndef __LOADER_H__
#define __LOADER_H__

#include <vector>
#include <string>
#include <glad/glad.h>

unsigned int loadTexture(char const* path);
unsigned int loadCubemap(std::vector<std::string> faces);

#endif