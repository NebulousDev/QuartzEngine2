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

layout(binding = 1) uniform sampler2D transmittanceLUT;
layout(binding = 2) uniform sampler2D scatteringLUT;

/* Intersection Functions */

float RaySphere(vec3 rayPos, vec3 rayDir, vec3 center, float radius)
{
	vec3 subPos = rayPos - center;

	float b = dot(subPos, rayDir);
	float c = dot(subPos, subPos) - radius * radius;

	if (c > 0.0f && b > 0.0)
		return -1.0;

	float quad = b*b - c;

	if (quad < 0.0)
		return -1.0;

	if (quad > b*b)
		return (-b + sqrt(quad));	
	else
		return -b - sqrt(quad);
}

/* LUT Functions */

vec3 LookupTransmittance(vec3 pos, vec3 sunDir)
{
	float height = length(pos);
    vec3 up = pos / height;
	float sunCosZenithAngle = dot(sunDir, up);

	float u = 0.5 + (0.5 * sunCosZenithAngle);
	float v = max(0.0, min((height - groundRadius)/(spaceRadius - groundRadius), 1.0));

    return texture(transmittanceLUT, vec2(u,v)).rgb;
}

vec3 LookupScattering(vec3 pos, vec3 sunDir)
{
	float height = length(pos);
    vec3 up = pos / height;
	float sunCosZenithAngle = dot(sunDir, up);
	
	float u = 0.5 + (0.5 * sunCosZenithAngle);
	float v = max(0.0, min((height - groundRadius)/(spaceRadius - groundRadius), 1.0));

    return texture(scatteringLUT, vec2(u,v)).rgb;
}

/* Atmosphere Property Functions */

float RayleighPhase(float cosAngle)
{
	return (3 * (1 + cosAngle * cosAngle)) / (16.0 * PI);
}

float MiePhase(float cosAngle)
{
	// Cornette-Shanks
	const float g = 0.8;
	const float g2 = g * g;
	const float coeff = 3.0 / (8.0 * PI);
	const float num = (1.0 - g2) * (1.0 + cosAngle * cosAngle);
	const float denom = (2 + g2) * pow((1 + g2 - 2 * g * cosAngle), (3.0 / 2.0));

	return coeff * (num / denom);
}

const float isoPhase = 1.0 / (4.0 * PI);

float RayleighDensity(float height)
{
	return exp(-height / 8.0); //8km?
}

float MieDensity(float height)
{
	return exp(-height / 1.2); //1.2km?
}

float OzoneDensity(float height)
{
	return max(0, 1.0 - (abs(height - 25.0) / 15.0));
}

void CalcScatterValues(vec3 pos, out vec3 outRayeigh, out float outMie, out vec3 outExtinction)
{
	const float height = (length(pos) - groundRadius) * KM;

	const float rayleighDensity = RayleighDensity(height);
	const float mieDensity = MieDensity(height);
	const float ozoneDensity = OzoneDensity(height);

	const float rayleighAbsorb = atmosphere.rayleighAbsorbtion * rayleighDensity;
	const float mieAbsorb = atmosphere.mieAbsorbtion * mieDensity;
	const vec3 ozoneAbsorb = atmosphere.ozoneAbsorbtion * ozoneDensity;

	outRayeigh = atmosphere.rayleighScattering * rayleighDensity;
	outMie = atmosphere.mieScattering * mieDensity;
	outExtinction = outRayeigh + rayleighAbsorb + outMie + mieAbsorb + ozoneAbsorb;
}

/* Atmosphere Functions */

const int numScatteringSteps = 32;
vec3 RaymarchScattering(vec3 pos, vec3 rayDir, vec3 sunDir, float numSteps)
{
    float cosTheta = dot(rayDir, sunDir);
    
	float atmoDist = RaySphere(pos, rayDir, vec3(0), spaceRadius);
	float groundDist = RaySphere(pos, rayDir, vec3(0), groundRadius);

	float tMax = atmoDist;
	if (groundDist > 0.0) {
		tMax = groundDist;
	}

	float miePhaseValue = MiePhase(cosTheta);
	float rayleighPhaseValue = RayleighPhase(-cosTheta);
    
    vec3 lum = vec3(0.0);
    vec3 transmittance = vec3(1.0);
    float t = 0.0;
    for (float i = 0.0; i < numSteps; i += 1.0) {
        float newT = ((i + 0.3)/numSteps)*tMax;
        float dt = newT - t;
        t = newT;
        
        vec3 newPos = pos + t*rayDir;
        
        vec3 rayleighScattering, extinction;
        float mieScattering;
        CalcScatterValues(newPos, rayleighScattering, mieScattering, extinction);
        
        vec3 sampleTransmittance = exp(-dt*extinction);

        vec3 sunTransmittance = LookupTransmittance(newPos, sunDir);
        vec3 psiMS = LookupScattering(newPos, sunDir);
        
        vec3 rayleighInScattering = rayleighScattering*(rayleighPhaseValue*sunTransmittance + psiMS);
        vec3 mieInScattering = mieScattering*(miePhaseValue*sunTransmittance + psiMS);
        vec3 inScattering = (rayleighInScattering + mieInScattering);

        // Integrated scattering within path segment.
        vec3 scatteringIntegral = (inScattering - inScattering * sampleTransmittance) / extinction;

        lum += scatteringIntegral*transmittance;
        
        transmittance *= sampleTransmittance;
    }

    return lum;
}

float safeacos(const float x) {
    return acos(clamp(x, -1.0, 1.0));
}

float QuadraticVCoord(float v)
{
	float lat = (v * PI) / 2.0;
	float f = sqrt(abs(lat) / (PI / 2.0));
	return 0.5 + 0.5 * sign(lat) * f;
}

void main()
{
	float azimuthAngle = (inUV.x - 0.5) * 2.0 * PI;
    // Non-linear mapping of altitude. See Section 5.3 of the paper.
    float adjV;
    if (inUV.y < 0.5) {
		float coord = 1.0 - 2.0 * inUV.y;
		adjV = -coord*coord;
	} else {
		float coord = inUV.y * 2.0 - 1.0;
		adjV = coord*coord;
	}

	float camHeight = groundRadius + atmosphere.cameraPos.y / (10.0 * KM);
	vec3 cameraPos = vec3(0.0, camHeight, 0.0);

    float height = length(cameraPos);
    vec3 up = cameraPos / height;
    float horizonAngle = safeacos(sqrt(height * height - groundRadius * groundRadius) / height) - 0.5 * PI;
    float altitudeAngle = adjV*0.5*PI - horizonAngle;
    
    float cosAltitude = cos(altitudeAngle);
    vec3 rayDir = vec3(cosAltitude*sin(azimuthAngle), sin(altitudeAngle), -cosAltitude*cos(azimuthAngle));
    
    float sunAltitude = (0.5*PI) - acos(dot(normalize(atmosphere.suns[0].dir), up));
    vec3 sunDir = vec3(0.0, sin(sunAltitude), -cos(sunAltitude));
    
    vec3 lum = RaymarchScattering(cameraPos, rayDir, sunDir, float(numScatteringSteps));

	fragOut = vec4(lum, 1.0);
}