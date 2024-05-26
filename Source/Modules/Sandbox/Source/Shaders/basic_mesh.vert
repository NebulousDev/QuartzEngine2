#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
//layout(location = 2) in vec3 inTangent;
//layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outUV;

layout(set = 0, binding = 0) uniform TransformUBO
{
	mat4 model;
	mat4 view;
	mat4 proj;
}
transform;

void main()
{
    outPosition = vec3(transform.model * vec4(inPosition, 1.0));
    outNormal = inNormal;
    outUV = inUV;

	gl_Position = (transform.proj * transform.view * transform.model) * vec4(inPosition, 1.0);
}