#version 450
#extension GL_ARB_separate_shader_objects : enable
//#extension GL_KHR_vulkan_glsl : enable

float signedDistanceSphere(vec3 point, vec3 center, float radius)
{
	return length(center - point) - radius;
}

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 fragOut;

const vec3 outerSphereCenter = vec3(0,0,5);
const vec3 innerSphereCenter = vec3(0,0,5);
const float outerSphereRadius = 10.0;
const float innerSphereRadius = 1.0;

const int maxMarches = 50;

const float width = 1280.0;
const float height = 720.0;
const float aspect = width / height;

void main()
{
	vec3 cameraPos = vec3(0.5,0.5,0);
	vec3 rayDir = vec3(inUV, 1.0) - cameraPos;
	rayDir.x *= aspect;

	vec3 point = rayDir;
	float totalDist = 0;
	int marches = maxMarches;

	rayDir = normalize(rayDir);

	for(int i = 0; i < maxMarches; i++)
	{
		float dist = signedDistanceSphere(point, innerSphereCenter, innerSphereRadius);
		totalDist += dist;

		if(dist < 0.001)
		{
			marches = i;
			break;
		}

		point += rayDir * dist;
	}

	totalDist = totalDist / float(marches);
	fragOut = vec4(totalDist, totalDist, totalDist, 1.0);
}