#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 mvp;
uniform mat4 model;

out vec2 TexCoords;

void main()
{
	TexCoords = (model * vec4(aPos.x, 0.0, aPos.y, 1.0)).xz;
	gl_Position = mvp * vec4(aPos.x, 0.0, aPos.y, 1.0);
}