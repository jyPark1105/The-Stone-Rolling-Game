// #version 330 core
// out vec4 FragColor;

// in vec2 TexCoords;

// uniform sampler2D circle;

// void main()
// {    
//     FragColor = texture(circle, TexCoords);
// }

#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

layout(std140) uniform Light {
    vec4 lightPositions[4];
    vec4 lightColors[4];
};

layout(std140) uniform CamPos {
    vec3 camPos;
};

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;

vec3 BlinnPhong(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor)
{
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(camPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // simple attenuation
    // float max_distance = 1.5;
    float distance = length(lightPos - fs_in.FragPos);
    float attenuation = 1.0 / sqrt(distance);
    
    diffuse *= attenuation;
    specular *= attenuation;
    
    return diffuse + specular;
}

void main()
{           
    vec3 color = pow(texture(texture_diffuse1, fs_in.TexCoords).rgb, vec3(2.2));
    vec3 normal = texture(texture_normal1, fs_in.TexCoords).rgb;
    normal = normalize(fs_in.TBN * (normal * 2.0 - 1.0));
    vec3 lighting = vec3(0.0);
    for(int i = 0; i < 4; ++i)
        lighting += BlinnPhong(normalize(normal), fs_in.FragPos, lightPositions[i].xyz, lightColors[i].xyz);
    color *= lighting;
    
    color = pow(color, vec3(1.0/2.2));
    FragColor = vec4(color, 1.0);
}

