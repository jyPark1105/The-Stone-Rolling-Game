#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;


uniform mat4 model;


layout(std140) uniform Matrices {
    mat4 vp;
};

void main()
{
	gl_Position = vp * model * vec4(aPos, 1.0);
}