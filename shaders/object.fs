// #version 430 core

// in vec2 TexCoords;
// in vec3 FragPos;
// in vec3 Normal;
// in mat3 TBN;

// out vec4 FragColor;

// uniform vec3 lightPos;
// uniform vec3 viewPos;

// uniform sampler2D texture_diffuse1;
// uniform sampler2D texture_normal1;
// uniform sampler2D texture_specular1;
// uniform sampler2D texture_ao1;

// vec3 getNormalFromMap() {
//     vec3 tangentNormal = texture(texture_normal1, TexCoords).rgb;
//     tangentNormal = tangentNormal * 2.0 - 1.0;
//     return normalize(TBN * tangentNormal);
// }

// void main()
// {
//     vec3 albedo = texture(texture_diffuse1, TexCoords).rgb * 2.0;   // ��¦ ���
//     float ao = max(texture(texture_ao1, TexCoords).r, 0.6);         // �ּ� ����
//     float specularStrength = max(texture(texture_specular1, TexCoords).r, 0.3);

//     vec3 N = getNormalFromMap();
//     vec3 V = normalize(viewPos - FragPos);
//     vec3 L = normalize(lightPos - FragPos);
//     vec3 H = normalize(V + L);

//     vec3 lightColor = vec3(1.0);
//     vec3 radiance = lightColor * 3.0; // ���� 5.0 �� 3.0���� ����

//     float diff = max(dot(N, L), 0.0);
//     vec3 diffuse = diff * albedo;

//     float spec = pow(max(dot(N, H), 0.0), 32.0);
//     vec3 specular = spec * specularStrength * lightColor;

//     vec3 ambient = albedo * ao * 0.2; // ambient ����

//     vec3 result = (ambient + diffuse + specular) * radiance;

//     FragColor = vec4(result, 1.0);
// }







#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
//in mat3 TBN;
in vec3 Normal;

// material parameters
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
//uniform sampler2D metallicMap;
uniform sampler2D texture_roughness1;
uniform sampler2D texture_ao1;

// lights
layout(std140) uniform Light {
    vec4 lightPositions[4];
    vec4 lightColors[4];
};

layout(std140) uniform CamPos {
    vec3 camPos;
};


const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anyways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(texture_normal1, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    T = normalize(T - N * dot(N, T));
    //vec3 B  = -normalize(cross(N, T));
    vec3 B = cross(N, T);
    if (dot(cross(T, B), N) < 0.0) B = -B;
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
void main()
{		
    vec3 albedo     = pow(texture(texture_diffuse1, TexCoords).rgb, vec3(2.2));
    //float metallic  = texture(metallicMap, TexCoords).r;
    float metallic = 0.0;
    float roughness = texture(texture_roughness1, TexCoords).r;
    float ao        = texture(texture_ao1, TexCoords).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - WorldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    //F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) 
    {
        vec3 L, radiance;
        float distance;
        // calculate per-light radiance
        if (lightPositions[i].w == 0)
        {
            L = normalize(lightPositions[i].xyz);
            radiance = lightColors[i].xyz * 60.0;
        }
        else{
            L = normalize(lightPositions[i].xyz - WorldPos);
            distance = length(lightPositions[i].xyz - WorldPos);
            float attenuation = 1.0 / (distance * distance);
            radiance = lightColors[i].xyz * 60.0 * attenuation;
        }
        vec3 H = normalize(V + L);

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator * 0.5;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;	  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        


        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   
    
    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
    //FragColor = vec4(albedo,1.0);
    //FragColor = vec4(texture(texture_normal1, TexCoords).xyz, 1.0);
}
