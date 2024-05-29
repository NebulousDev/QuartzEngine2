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

struct PointLight
{
    vec4    position;
    vec3    color;
    float   intensity;
};

layout(std140, set = 0, binding = 0) uniform SceneUBO
{
	vec3        cameraPosition;
    uint        pointLightCount;
    PointLight  pointLights[10];
}
scene;

layout(set = 0, binding = 2) uniform sampler2D diffuseTexture;
layout(set = 0, binding = 3) uniform sampler2D normalTexture;
layout(set = 0, binding = 4) uniform sampler2D roughnessTexture;
layout(set = 0, binding = 5) uniform sampler2D metallicTexture;
layout(set = 0, binding = 6) uniform sampler2D aoTexture;

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
    vec3 viewPos = scene.cameraPosition;
    vec3 viewDir = normalize(viewPos - inPosition);

    vec3 color          = texture(diffuseTexture, inTexCoord).rgb;
    vec3 normal         = texture(normalTexture, inTexCoord).rgb;
    float roughness     = texture(roughnessTexture, inTexCoord).r;
    float metallic      = texture(metallicTexture, inTexCoord).r;
    float ao            = texture(aoTexture, inTexCoord).r;

    normal = normal * 2.0 - 1.0;
    normal = normalize(inTBN * normal);    

    vec3 F0 = mix(vec3(0.04), color, metallic);

    vec3 totalLight = vec3(0);

    // Point lights
    for(uint i = 0; i < scene.pointLightCount; i++)
    {
        const PointLight pointLight = scene.pointLights[i];
        const vec3 lightPos         = pointLight.position.xyz;
        const vec3 lightColor       = pointLight.color * pointLight.intensity;

        vec3 lightDir       = normalize(lightPos - inPosition);
        //lightDir            = mix(lightDir, sunDirs[i], isSun[i]);
        vec3 halfwayDir     = normalize(lightDir + viewDir);
        float distance      = length(lightPos - inPosition);
        float attenuation   = 1.0 / (distance * distance);
        //attenuation         = mix(attenuation, 1.0, isSun[i]);
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
    
    vec3 ambient = vec3(0.001) * color * ao;
    vec3 outColor = ambient + totalLight;

	fragOut = vec4(outColor, 1.0);
}