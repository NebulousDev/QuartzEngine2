#version 450
#extension GL_ARB_separate_shader_objects : enable

/* Constants */

const float PI = 3.14159265358;
const float KM = 1000.0;

const vec3 spaceCenter = vec3(0,0,0);
const vec3 groundCenter = vec3(0,0,0);
const float spaceRadius = 6.460;
const float groundRadius = 6.360;

/* Uniforms */

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 fragOut;

struct Sun
{
	vec3 dir;
	float intensity;
};

layout(set = 0, binding = 0) uniform AtmosphereUBO
{
	vec3 rayleighScattering;
	float rayleighAbsorbtion;
	float mieScattering;
	float mieAbsorbtion;
	float ozoneScattering;
	vec3 ozoneAbsorbtion;
	Sun suns[2];

	vec3 cameraPos;
	vec3 viewDir;
	float width;
	float height;
}
atmosphere;

/* Intersection Functions */

float RaySphere(vec3 ro, vec3 rd, vec3 ttt, float rad)
{
	float b = dot(ro, rd);
	float c = dot(ro, ro) - rad*rad;	
	if (c > 0.0f && b > 0.0)
		return -1.0;	
	float discr = b*b - c;	
	if (discr < 0.0)
		return -1.0;	
	// Special case: inside sphere, use far discriminant
	if (discr > b*b)
		return (-b + sqrt(discr));	
	return -b - sqrt(discr);
}

/* Atmosphere Property Functions */

float CalcRayleighDensity(float height)
{
	return exp(-height / 8.0);
}

float CalcMieDensity(float height)
{
	return exp(-height / 1.2);
}

float CalcOzoneDensity(float height)
{
	return max(0.0, 1.0 - (abs(height - 25.0) / 15.0));
}

void CalcScatterValues(vec3 pos, out vec3 outRayeigh, out float outMie, out vec3 outExtinction)
{
	const float height = (length(pos) - groundRadius) * KM; //needs to be reworked

	const float rayleighDensity = CalcRayleighDensity(height);
	const float mieDensity = CalcMieDensity(height);
	const float ozoneDensity = CalcOzoneDensity(height);

	const float rayleighAbsorb = atmosphere.rayleighAbsorbtion * rayleighDensity;
	const float mieAbsorb = atmosphere.mieAbsorbtion * mieDensity;
	const vec3 ozoneAbsorb = atmosphere.ozoneAbsorbtion * ozoneDensity;

	outRayeigh = atmosphere.rayleighScattering * rayleighDensity;
	outMie = atmosphere.mieScattering * mieDensity;
	outExtinction = outRayeigh + rayleighAbsorb + outMie + mieAbsorb + ozoneAbsorb;
}

/* Atmosphere Functions */

vec3 Transmittance(vec3 pos, vec3 sunDir)
{
	const float distToGround = RaySphere(pos, sunDir, groundCenter, groundRadius);

	if(distToGround > 0.0) // Sun is below the horizon?
		return vec3(0);

	const float distToSpace = RaySphere(pos, sunDir, spaceCenter, spaceRadius);

	vec3 transmittance = vec3(1.0);

	float time = 0;
	const float steps = 40.0;
	for(float i = 0.0; i < steps; i += 1.0)
	{
		const float newTime = ((i + 0.3)/steps) * distToSpace; // +0.3??
		const float deltaTime = newTime - time;
		time = newTime;

		vec3 newPos = pos + time * sunDir;

		vec3 rayeigh; // why these?
		float mie;
		vec3 extinction;
		CalcScatterValues(newPos, rayeigh, mie, extinction);

		transmittance *= exp(-extinction * deltaTime);
	}

	return transmittance;
}

/* Main Functions */

float safeacos(const float x)
{
    return acos(clamp(x, -1.0, 1.0));
}

void main()
{
    float sunCosTheta = (2.0 * inUV.x) - 1.0;
    float sunTheta = safeacos(sunCosTheta);
    float height = mix(groundRadius, spaceRadius, inUV.y);
    
    vec3 pos = vec3(0.0, height, 0.0); 
    vec3 sunDir = normalize(vec3(0.0, sunCosTheta, -sin(sunTheta)));
    
    fragOut = vec4(Transmittance(pos, sunDir), 1.0);
}