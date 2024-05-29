#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

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

const float shininess = 8;

void main()
{
    vec3 normal = texture(normalTexture, inTexCoord).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(inTBN * normal);

    vec3 viewPos = scene.cameraPosition;

    vec3 totalLight = vec3(0);
    for(uint i = 0; i < scene.pointLightCount; i++)
    {
        const PointLight pointLight = scene.pointLights[i];
        const vec3 lightPos         = pointLight.position.xyz;
        const vec3 lightColor       = pointLight.color * pointLight.intensity;

        vec3 lightDir   = normalize(lightPos - inPosition);
        vec3 viewDir    = normalize(viewPos - inPosition);
        vec3 halfwayDir = normalize(lightDir + viewDir);

        float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
        vec3 specular = lightColor * spec;

        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        totalLight += (diffuse * lightColor) + specular;
    }

    vec3 color = texture(diffuseTexture, inTexCoord).rgb;
	fragOut = vec4(color * totalLight, 1.0);
}