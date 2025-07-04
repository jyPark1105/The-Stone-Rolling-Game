#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
    vec3 Normal;
} fs_in;

//in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;

layout(std140) uniform Light {
    vec4 lightPositions[4];
    vec4 lightColors[4];
};

layout(std140) uniform CamPos {
    vec3 camPos;
};

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(texture_normal1, fs_in.TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(fs_in.FragPos);
    vec3 Q2  = dFdy(fs_in.FragPos);
    vec2 st1 = dFdx(fs_in.TexCoords);
    vec2 st2 = dFdy(fs_in.TexCoords);

    vec3 N   = normalize(fs_in.Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    T = normalize(T - N * dot(N, T));
    vec3 B = cross(N, T);
    if (dot(cross(T, B), N) < 0.0) B = -B;
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

vec3 BlinnPhong(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor, float specular_coef)
{
    // diffuse
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(camPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 8.0);
    vec3 specular = spec * lightColor * specular_coef;    
    // simple attenuation
    float max_distance = 1.5;
    float distance = length(lightPos - fragPos);

    float attenuation = 1.0 / (distance);
    
    diffuse *= attenuation;
    specular *= attenuation * attenuation;
    
    return diffuse + specular;
}

void main()
{           
    vec3 color = pow(texture(texture_diffuse1, fs_in.TexCoords).rgb, vec3(2.2));
    vec3 normal = texture(texture_normal1, fs_in.TexCoords).rgb;
    normal = normalize(fs_in.TBN * (normal * 2.0 - 1.0));  // this normal is in tangent space
    //normal = fs_in.Normal;

    float specular = texture(texture_specular1, fs_in.TexCoords).r;
    vec3 lighting = vec3(0.0);
    for(int i = 0; i < 4; ++i)
        lighting += BlinnPhong(normal, fs_in.FragPos, lightPositions[i].xyz, lightColors[i].xyz, specular);
    color *= (lighting);
    
    color = pow(color, vec3(1.0/2.2));
    FragColor = vec4(color, 1.0);
    //FragColor = vec4(texture(texture_normal1, fs_in.TexCoords).xyz, 1.0);
}

