// #version 430 core
// layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 aNormal;
// layout (location = 2) in vec2 aTexCoords;
// layout (location = 3) in vec3 aTangent;
// layout (location = 4) in vec3 aBitangent;

// uniform mat4 M;
// uniform mat4 V;
// uniform mat4 P;

// out vec3 FragPos;
// out vec2 TexCoords;
// out vec3 Normal;
// out mat3 TBN;

// void main()
// {
//     vec3 T = normalize(mat3(M) * aTangent);
//     vec3 B = normalize(mat3(M) * aBitangent);
//     vec3 N = normalize(mat3(M) * aNormal);
//     TBN = mat3(T, B, N);

//     TexCoords = aTexCoords;
//     FragPos = vec3(M * vec4(aPos, 1.0));
//     Normal = N;

//     gl_Position = P * V * vec4(FragPos, 1.0);
// }

#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec3 WorldPos;
out vec2 TexCoords;
out vec3 Normal;

uniform sampler2D texture_displacement1;

void main()
{
    //float height = texture(texture_displacement1, TexCoords).r;
    //vec3 displacePos = aPos + aNormal * height * 0.1;
    TexCoords = aTexCoords;
    WorldPos = vec3(M * vec4(aPos, 1.0));
    //WorldPos = displacePos;
    Normal = transpose(inverse(mat3(M))) * aNormal;
    

    gl_Position = P * V * vec4(WorldPos, 1.0);
}