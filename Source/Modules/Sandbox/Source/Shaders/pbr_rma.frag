#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

#define PI 3.14159265

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in mat3 inTBN;

layout(location = 0) out vec4 fragOut;

layout(std140, set = 0, binding = 0) uniform SceneUBO
{
	vec3 cameraPosition;
}
scene;

layout(set = 0, binding = 2) uniform sampler2D diffuseTexture;
layout(set = 0, binding = 3) uniform sampler2D normalTexture;
layout(set = 0, binding = 4) uniform sampler2D rmaTexture;

vec3 PBR_FresnelSchlick(float cosTheta, vec3 metallic)
{
    return metallic + (1.0 - metallic) * pow(1.0 - cosTheta, 5.0);
}

float PBR_GGX(vec3 normal, vec3 halfwayDir, float roughness)
{
    float rough2 = roughness * roughness;
    float nDotH  = max(dot(normal, halfwayDir), 0.0);
    float nDotH2 = nDotH * nDotH;
	
    float num    = rough2;
    float denom  = (nDotH2 * (rough2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return num / denom;
}

float PBR_SchlickGGX(float nDotV, float roughness)
{
    float num   = nDotV;
    float denom = nDotV * (1.0 - roughness) + roughness;
    return num / denom;
}
  
float PBR_Smith(float nDotV, float nDotL, float roughness)
{
    float ggx0 = PBR_SchlickGGX(nDotV, roughness);
    float ggx1 = PBR_SchlickGGX(nDotL, roughness);
    return ggx0 * ggx1;
}

void main()
{
    const vec3 lightPos     = vec3(-15.0, 25.0, 15.0);
    const vec3 lightColor   = vec3(1,1,1) * 500;

    vec3 viewPos = scene.cameraPosition;
    vec3 viewDir = normalize(viewPos - inPosition);

    vec3 color  = texture(diffuseTexture, inTexCoord).rgb;
    vec3 normal = texture(normalTexture, inTexCoord).rgb;
    vec3 rma    = texture(rmaTexture, inTexCoord).rgb;

    normal = normal * 2.0 - 1.0;
    normal = normalize(inTBN * normal);

    float roughness   = rma.r;
    float metallic    = rma.g + 0.01;
    float ao          = rma.b;

    vec3 F0 = mix(vec3(0.04), color, metallic);

    vec3 totalLight = vec3(0);
    for(uint i = 0; i < 1; i++)
    {
        vec3 lightDir       = normalize(lightPos - inPosition);
        vec3 halfwayDir     = normalize(lightDir + viewDir);
        float distance      = length(lightPos - inPosition);
        float attenuation   = 1.0 / (distance * distance);
        vec3 radiance       = lightColor * attenuation;

        float nDotL         = max(dot(normal, lightDir), 0.0);
        float nDotV         = max(dot(normal, viewDir), 0.0);
        float hDotV         = max(dot(halfwayDir, viewDir), 0.0);

        float normalDist    = PBR_GGX(normal, halfwayDir, roughness);
        float geometry      = PBR_Smith(nDotV, nDotL, roughness);
        vec3 metallicSpec   = PBR_FresnelSchlick(hDotV, F0);

        // Cook-Torrance BRDF
        vec3 numerator      = normalDist * geometry * metallicSpec;
        float denominator   = 4.0 * nDotV * nDotL + 0.0001;
        vec3 specular       = numerator / denominator;

        vec3 outSpecular    = metallicSpec;
        vec3 outDiffuse     = (vec3(1.0) - outSpecular) * (1.0 - metallic);
        
        totalLight          += (outDiffuse * color / PI + specular) * radiance * nDotL;
    }
    
    vec3 ambient = vec3(0.02) * color * ao;
    vec3 outColor = ambient + totalLight;

    outColor = outColor / (outColor + vec3(1.0));
    outColor = pow(outColor, vec3(1.0/2.2));

	fragOut = vec4(outColor, 1.0);
}