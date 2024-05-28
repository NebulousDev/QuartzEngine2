#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec4 outTangent;
layout(location = 3) out vec2 outTexCoord;
layout(location = 4) out mat3 outTBN;

layout(std140, set = 0, binding = 1) uniform TransformUBO
{
	mat4 model;
	mat4 view;
	mat4 proj;
}
transform;

void main()
{
    vec3 T = normalize(vec3(transform.model * vec4(inTangent.xyz, 0.0)));
    vec3 B = normalize(vec3(transform.model * vec4(cross(inNormal, inTangent.xyz) * inTangent.w, 0.0)));
 	vec3 N = normalize(vec3(transform.model * vec4(inNormal, 0.0)));

    outNormal = inNormal;
	outTBN = mat3(T, B, N);
    outTexCoord = inTexCoord;

    outPosition = vec3(transform.model * vec4(inPosition, 1.0));

	gl_Position = (transform.proj * transform.view * transform.model) * vec4(inPosition, 1.0);
}