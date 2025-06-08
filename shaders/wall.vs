#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in mat4 aInstancePos;

layout(std140) uniform Matrices {
    mat4 vp;
};

uniform mat4 model;

out vec2 TexCoords;
out vec3 WorldPos;
//out mat3 TBN;
out vec3 Normal;


void main()
{
	mat4 tmp = model * aInstancePos;
	TexCoords = aTexCoords * 0.1;
    WorldPos = vec3(tmp * vec4(aPos, 1.0));
	//TBN = mat3(aTangent, aBitangent, aNormal);
	Normal = transpose(inverse(mat3(tmp))) * aNormal;

    gl_Position =  vp * vec4(WorldPos, 1.0);
}
