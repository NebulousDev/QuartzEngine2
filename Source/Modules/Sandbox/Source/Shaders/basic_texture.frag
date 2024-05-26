#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec4 fragOut;

layout(set = 0, binding = 1) uniform sampler2D diffuseTexture;

const vec3 lightPos = vec3(-15.0, 55.0, 15.0);
const vec3 viewPos = vec3(-10.0, 25.0, 1.0);
const float shininess = 8;
const vec3 lightColor = vec3(1,1,1);

void main()
{
    vec3 lightDir   = normalize(lightPos - inPosition);
    vec3 viewDir    = normalize(viewPos - inPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = pow(max(dot(inNormal, halfwayDir), 0.0), shininess);
    vec3 specular = lightColor * spec;

    float diff = max(dot(inNormal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 lighting = (diffuse * lightColor) + specular;
    //vec3 color = texture(diffuseTexture, vec2(0,0)).rgb;
    vec3 color = texture(diffuseTexture, inUV).rgb;
	fragOut = vec4(color * lighting, 1.0);
}