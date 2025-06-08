//#version 330 core
//layout (location = 0) in vec3 aPos;
//
//out vec2 TexCoords;
//
//layout(std140) uniform Matrices {
//    mat4 vp;
//};
//
//uniform mat4 model;
//
//void main()
//{
//	TexCoords = aPos.xz;
//	gl_Position = vp * model * vec4(aPos, 1.0);
//}

#version 330 core
layout (location = 0) in vec3 aPos;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} vs_out;

layout(std140) uniform Matrices {
    mat4 vp;
};

uniform mat4 model;

void main()
{
	vs_out.FragPos = vec3(model * vec4(aPos, 1.0)); 
	gl_Position = vp * vec4(vs_out.FragPos, 1.0);  
    vs_out.TexCoords = aPos.xz;
    
    vec3 T = vec3(1.0, 0.0, 0.0);
    vec3 N = vec3(0.0, 0.0, 1.0);
    vec3 B = vec3(0.0, 1.0, 0.0);
	vs_out.TBN = mat3(T, B, N);
}