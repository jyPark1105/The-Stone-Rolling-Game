#version 430 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube cb;

void main()
{    
    vec3 color = texture(cb, TexCoords).rgb * 0.1;
    FragColor = vec4(color, 1.0);
}
