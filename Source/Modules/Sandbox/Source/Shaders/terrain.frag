#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in float inHeight;

layout(location = 0) out vec4 fragOut;

void main()
{
	float height = inHeight;
	vec3 hColor = vec3(height);

	if(height < 0.0)
	{
		hColor = vec3(0.1f, 0.2f, 0.9f);
	}
	else
	{
		hColor = vec3(0.0f, 1.0f, 0.1f);
	}

	fragOut = vec4(hColor * (height * 0.7 + 0.3), 1.0);
}