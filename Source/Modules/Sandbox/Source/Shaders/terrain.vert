#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;

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
	float sampleX = inPosition.x; //* 0.95 + 0.05;
	float sampleY = inPosition.z; //* 0.95 + 0.05;

	float height = texture(perlinHeightmap, vec2(sampleX, sampleY)).x;
	outHeight = height;
	gl_Position = (ubo.proj * ubo.view * ubo.model) * vec4(inPosition.x, height, inPosition.z, 1.0);
}