#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 fragOut;

void main()
{
	fragOut = vec4(inUV, 1.0, 1.0);
}