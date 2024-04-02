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

/* LUT Functions */

vec3 LookupTransmittance(vec3 pos, vec3 sunDir)
{
	float height = length(pos);
    vec3 up = pos / height;
	float sunCosZenithAngle = dot(sunDir, up);

	float u = clamp(0.5 + (0.5 * sunCosZenithAngle), 0.0, 1.0);
	float v = max(0.0, min((height - groundRadius)/(spaceRadius - groundRadius), 1.0));

    return texture(transmittanceLUT, vec2(u,v)).rgb;
}

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

float CalcRayleighPhase(float cosAngle)
{
	return (3.0 * (1.0 + cosAngle * cosAngle)) / (16.0 * PI);
}
float CalcMiePhase(float cosAngle)
{
	// Cornette-Shanks
	const float g = 0.8;
	const float g2 = g * g;
	const float coeff = 3.0 / (8.0 * PI);
	const float num = (1.0 - g2) * (1.0 + cosAngle * cosAngle);
	const float denom = (2.0 + g2) * pow((1.0 + g2 - 2.0 * g * cosAngle), (3.0 / 2.0));

	return coeff * (num / denom);
}

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
	const float height = (length(pos) - groundRadius) * KM;

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

const float mulScattSteps = 20.0;
const int sqrtSamples = 8;

vec3 getSphericalDir(float theta, float phi)
{
     float cosPhi = cos(phi);
     float sinPhi = sin(phi);
     float cosTheta = cos(theta);
     float sinTheta = sin(theta);
     return vec3(sinPhi*sinTheta, cosPhi, sinPhi*cosTheta);
}

float safeacos(const float x)
{
    return acos(clamp(x, -1.0, 1.0));
}

// Calculates Equation (5) and (7) from the paper.
vec3 Scatter(vec3 pos, vec3 sunDir)
{
    vec3 lumTotal = vec3(0.0);
    vec3 fms = vec3(0.0);
    
    float invSamples = 1.0/float(sqrtSamples*sqrtSamples);
    for (int i = 0; i < sqrtSamples; i++) {
        for (int j = 0; j < sqrtSamples; j++) {
            // This integral is symmetric about theta = 0 (or theta = PI), so we
            // only need to integrate from zero to PI, not zero to 2*PI.
            float theta = PI * (float(i) + 0.5) / float(sqrtSamples);
            float phi = safeacos(1.0 - 2.0*(float(j) + 0.5) / float(sqrtSamples));
            vec3 rayDir = getSphericalDir(theta, phi);
            
            float atmoDist = RaySphere(pos, rayDir, vec3(0), spaceRadius);
            float groundDist = RaySphere(pos, rayDir, vec3(0), groundRadius);
            
            float tMax = atmoDist;
            if (groundDist > 0.0) {
                tMax = groundDist;
            }
            
            float cosTheta = dot(rayDir, sunDir);
    
            float miePhaseValue = CalcMiePhase(cosTheta);
            float rayleighPhaseValue = CalcRayleighPhase(-cosTheta);
            
            vec3 lum = vec3(0.0), lumFactor = vec3(0.0), transmittance = vec3(1.0);
            float t = 0.0;
            for (float stepI = 0.0; stepI < mulScattSteps; stepI += 1.0) {
                float newT = ((stepI + 0.3)/mulScattSteps)*tMax;
                float dt = newT - t;
                t = newT;

                vec3 newPos = pos + t*rayDir;

                vec3 rayleighScattering, extinction;
                float mieScattering;
                CalcScatterValues(newPos, rayleighScattering, mieScattering, extinction);

                vec3 sampleTransmittance = exp(-dt*extinction);
                
                // Integrate within each segment.
                vec3 scatteringNoPhase = rayleighScattering + mieScattering;
                vec3 scatteringF = (scatteringNoPhase - scatteringNoPhase * sampleTransmittance) / extinction;
                lumFactor += transmittance*scatteringF;
                
                // This is slightly different from the paper, but I think the paper has a mistake?
                // In equation (6), I think S(x,w_s) should be S(x-tv,w_s).
                vec3 sunTransmittance = LookupTransmittance(newPos, sunDir);

                vec3 rayleighInScattering = rayleighScattering*rayleighPhaseValue;
                float mieInScattering = mieScattering*miePhaseValue;
                vec3 inScattering = (rayleighInScattering + mieInScattering)*sunTransmittance;

                // Integrated scattering within path segment.
                vec3 scatteringIntegral = (inScattering - inScattering * sampleTransmittance) / extinction;

                lum += scatteringIntegral*transmittance;
                transmittance *= sampleTransmittance;
            }
            
            vec3 groundAlbedo = vec3(0.3);

            if (groundDist > 0.0) {
                vec3 hitPos = pos + groundDist*rayDir;
                if (dot(pos, sunDir) > 0.0) {
                    hitPos = normalize(hitPos)*groundRadius;
                    lum += transmittance*groundAlbedo*LookupTransmittance(hitPos, sunDir);
                }
            }
            
            fms += lumFactor*invSamples;
            lumTotal += lum*invSamples;
        }
    }

	return lumTotal / (1.0 - fms);
}

/* Main Functions */

void main()
{
	float sunCosTheta = (2.0 * inUV.x) - 1.0;
    float sunTheta = safeacos(sunCosTheta);
    float height = mix(groundRadius, spaceRadius, inUV.y);
    
    vec3 pos = vec3(0.0, height, 0.0); 
    vec3 sunDir = normalize(vec3(0.0, sunCosTheta, -sin(sunTheta)));

	fragOut = vec4(Scatter(pos, sunDir), 1.0);
}