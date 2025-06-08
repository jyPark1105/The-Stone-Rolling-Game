#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D flr;

void main()
{    
    FragColor = texture(flr, TexCoords);
}
