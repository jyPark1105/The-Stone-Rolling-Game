#version 430 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 V;
uniform mat4 P;


void main()
{
    TexCoords = aPos;
    vec4 pos = P * V * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  