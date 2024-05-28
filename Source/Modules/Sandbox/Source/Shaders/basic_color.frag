#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 fragOut;

layout(std140, set = 0, binding = 0) uniform SceneUBO
{
	vec3 cameraPosition;
}
scene;

layout(std140, set = 0, binding = 2) uniform Material
{
    vec3 color;
};

const vec3 lightPos = vec3(-15.0, 55.0, 15.0);
const float shininess = 8;
const vec3 lightColor = vec3(1,1,1);

void main()
{
    vec3 viewPos = scene.cameraPosition;

    vec3 lightDir   = normalize(lightPos - inPosition);
    vec3 viewDir    = normalize(viewPos - inPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = pow(max(dot(inNormal, halfwayDir), 0.0), shininess);
    vec3 specular = lightColor * spec;

    float diff = max(dot(inNormal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 lighting = (diffuse * lightColor) + specular;
	fragOut = vec4(color * lighting, 1.0);
}