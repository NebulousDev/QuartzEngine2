#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
//layout(location = 2) in vec3 inTangent;
//layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out float outHeight;

layout(set = 0, binding = 0) uniform TransformUBO
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(binding = 1) uniform sampler2D perlinHeightmap;

void main()
{
	float height = texture(perlinHeightmap, inPosition.xz).x;
	//float height = 0.0f;
	outHeight = height;
	gl_Position = (ubo.proj * ubo.view * ubo.model) * vec4(inPosition.x, height, inPosition.z, 1.0);
}