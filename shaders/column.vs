#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 7) in mat4 aInstanceMatrix;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

layout(std140) uniform Matrices {
    mat4 vp;
};

void main()
{
	//TexCoords = aTexCoords;
	//gl_Position = vp * aInstanceMatrix * vec4(aPos, 1.0);

	vs_out.FragPos = vec3(aInstanceMatrix * vec4(aPos,1.0));
    vs_out.Normal = transpose(inverse(mat3(aInstanceMatrix))) * aNormal;
    vs_out.TexCoords = aTexCoords;
    gl_Position = vp * vec4(vs_out.FragPos, 1.0);
}