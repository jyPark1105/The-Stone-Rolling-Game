//TexCoords = aTexCoords;
//gl_Position = vp * model * aInstanceMatrix * vec4(aPos, 1.0);


#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 7) in mat4 aInstanceMatrix;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
    vec3 Normal;
} vs_out;

layout(std140) uniform Matrices {
    mat4 vp;
};

layout(std140) uniform Light {
    vec4 lightPositions[4];
    vec4 lightColors[4];
};

layout(std140) uniform CamPos {
    vec3 camPos;
};

uniform mat4 model;

void main()
{
	mat4 tmp = model * aInstanceMatrix;

    vs_out.FragPos = vec3(tmp * vec4(aPos, 1.0));
	gl_Position = vp * vec4(vs_out.FragPos, 1.0);
	mat3 normalMatrix = transpose(inverse(mat3(tmp))); 
    vs_out.Normal = normalize(normalMatrix * aNormal);
    vs_out.TexCoords = aTexCoords;
    
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    B = normalize(normalMatrix * aBitangent);
    
    vs_out.TBN = mat3(T, B, N);
}