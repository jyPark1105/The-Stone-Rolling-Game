#version 330 core
layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aNormal;
//layout (location = 2) in vec2 aTexCoords;

layout(std140) uniform Matrices {
    mat4 vp;
};

uniform mat4 model;
uniform mat3 normalMatrix;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;


void main()
{
	// TexCoords = aPos.xz;
	// gl_Position = vp * model * vec4(aPos, 1.0);

	TexCoords = 0.1 * aPos.xz;
    WorldPos = vec3(model * vec4(aPos, 1.0));
    Normal = normalMatrix * vec3(0.0, 1.0, 0.0);

    gl_Position =  vp * vec4(WorldPos, 1.0);
}
