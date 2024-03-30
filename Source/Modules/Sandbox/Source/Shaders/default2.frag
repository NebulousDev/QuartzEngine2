#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 inNormal;

layout(location = 0) out vec4 fragOut;

void main()
{
	fragOut = vec4(0.5, 0.5, 0.6, 1.0);
}