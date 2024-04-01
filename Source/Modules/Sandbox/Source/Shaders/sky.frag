#version 450
#extension GL_ARB_separate_shader_objects : enable

const float PI = 3.14159265358;
const float KM = 1000.0;

const vec3 outerSphereCenter = vec3(0,0,0);
const vec3 innerSphereCenter = vec3(0,0,0);
const float outerSphereRadius = 6.460;
const float innerSphereRadius = 6.360;

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
	float _pad0_, _pad1_;
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


//float RaySphere(vec3 rayStart, vec3 rayDir, vec3 sphereCenter, float radius)
//{
//	const float a = dot(rayStart, rayDir);
//	const vec3 toStart = rayStart - sphereCenter;
//	const float b = 2.0 * dot(rayDir, toStart);
//	const float c = dot(toStart, toStart) - (radius * radius);
//	const float quad = (b * b) - (4.0 * a * c);
//
//	if(quad < 0.0) return -1.0;
//	else return (-b - sqrt(quad)) / (2.0 * a);
//}

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
	const float height = (length(pos) - innerSphereRadius) * KM;

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

/* LUT Functions */

vec3 Transmittance(vec3 pos, vec3 sunDir)
{
	const float distToGround = RaySphere(pos, sunDir, innerSphereCenter, innerSphereRadius);

	if(distToGround > 0.0) // Sun is below the horizon?
		return vec3(0);

	const float distToSpace = RaySphere(pos, sunDir, outerSphereCenter, outerSphereRadius);

	vec3 finalTransmittance = vec3(1.0);

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

		finalTransmittance *= exp(-extinction * deltaTime);
	}

	return finalTransmittance;
}

void Luminance2ndOrder()
{

}

void MultipleScattering()
{
	const float steps = 8.0;
	const float multiSteps = 20.0;
	
	vec3 totalLuminance = vec3(0);
	vec3 fms = vec3(0);

	for(float i = 0; i < steps; i++)
	{
		for(float j = 0; j < steps; i++)
		{
			
		}
	}


}

/* Other Functions */

float SunAltitude(float time)
{
	return 0;
}

vec3 SunWithBloom(vec3 rayDir, vec3 sunDir)
{
	const float sunAngleWidth = 1.0 * (PI / 180.0);
	const float minSunCosTheta = cos(sunAngleWidth);
	const float cosTheta = dot(rayDir, sunDir);

	if (cosTheta >= minSunCosTheta) 
		return vec3(1.0);
		
	const float offset = minSunCosTheta - cosTheta;
	const float gaussianBloom = exp(-offset * 50000.0) * 0.5;
	const float invBloom = 1.0 / (0.02 + offset * 30.0) * 0.01;

	return vec3(gaussianBloom+invBloom);
}



const float mulScattSteps = 20.0;
const int sqrtSamples = 8;

vec3 getSphericalDir(float theta, float phi) {
     float cosPhi = cos(phi);
     float sinPhi = sin(phi);
     float cosTheta = cos(theta);
     float sinTheta = sin(theta);
     return vec3(sinPhi*sinTheta, cosPhi, sinPhi*cosTheta);
}

float safeacos(const float x) {
    return acos(clamp(x, -1.0, 1.0));
}

// Calculates Equation (5) and (7) from the paper.
void MulScattValues(vec3 pos, vec3 sunDir, out vec3 lumTotal, out vec3 fms)
{
    lumTotal = vec3(0.0);
    fms = vec3(0.0);
    
    float invSamples = 1.0/float(sqrtSamples*sqrtSamples);
    for (int i = 0; i < sqrtSamples; i++) {
        for (int j = 0; j < sqrtSamples; j++) {
            // This integral is symmetric about theta = 0 (or theta = PI), so we
            // only need to integrate from zero to PI, not zero to 2*PI.
            float theta = PI * (float(i) + 0.5) / float(sqrtSamples);
            float phi = safeacos(1.0 - 2.0*(float(j) + 0.5) / float(sqrtSamples));
            vec3 rayDir = getSphericalDir(theta, phi);
            
            float atmoDist = RaySphere(pos, rayDir, vec3(0), outerSphereRadius);
            float groundDist = RaySphere(pos, rayDir, vec3(0), innerSphereRadius);
            float tMax = atmoDist;
            if (groundDist > 0.0) {
                tMax = groundDist;
            }
            
            float cosTheta = dot(rayDir, sunDir);
    
            float miePhaseValue = MiePhase(cosTheta);
            float rayleighPhaseValue = RayleighPhase(-cosTheta);
            
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
                //vec3 sunTransmittance = getValFromTLUT(iChannel0, iChannelResolution[0].xy, newPos, sunDir);
				vec3 sunTransmittance = Transmittance(newPos, sunDir);
				
                vec3 rayleighInScattering = rayleighScattering*rayleighPhaseValue;
                float mieInScattering = mieScattering*miePhaseValue;
                vec3 inScattering = (rayleighInScattering + mieInScattering)*sunTransmittance;

                // Integrated scattering within path segment.
                vec3 scatteringIntegral = (inScattering - inScattering * sampleTransmittance) / extinction;

                lum += scatteringIntegral*transmittance;
                transmittance *= sampleTransmittance;
            }
            
            if (groundDist > 0.0) {
                vec3 hitPos = pos + groundDist*rayDir;
                if (dot(pos, sunDir) > 0.0) {
                    hitPos = normalize(hitPos)*innerSphereRadius;
                    //lum += transmittance*groundAlbedo*getValFromTLUT(iChannel0, iChannelResolution[0].xy, hitPos, sunDir);
					lum += transmittance * vec3(0.3) * Transmittance(hitPos, sunDir);
                }
            }
            
            fms += lumFactor*invSamples;
            lumTotal += lum*invSamples;
        }
    }
}

const int numScatteringSteps = 32;
vec3 RaymarchScattering(vec3 pos, vec3 rayDir,  vec3 sunDir, float numSteps)
{
    float cosTheta = dot(rayDir, sunDir);
    
	float atmoDist = RaySphere(pos, rayDir, vec3(0), outerSphereRadius);
	float groundDist = RaySphere(pos, rayDir, vec3(0), innerSphereRadius);
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

        vec3 sunTransmittance = Transmittance(newPos, sunDir);

		vec3 lum2 = vec3(0);
		vec3 fms = vec3(0);
		MulScattValues(rayDir, sunDir, lum2, fms);
		lum2 /= (1.0 - fms);
		
        vec3 psiMS = lum2;
        
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

vec3 jodieReinhardTonemap(vec3 c){
    // From: https://www.shadertoy.com/view/tdSXzD
    float l = dot(c, vec3(0.2126, 0.7152, 0.0722));
    vec3 tc = c / (c + 1.0);
    return mix(c / (l + 1.0), tc, tc);
}

/* Main Functions */

void main()
{
	float aspect = atmosphere.width / atmosphere.height;

	float cameraY = innerSphereRadius + 0.0002;
	vec3 cameraPos = vec3(0.0,cameraY,0.0);
	vec2 coords = 2.0 * inUV - vec2(1.0);
	vec3 rayDir = normalize(vec3(coords.x * aspect, coords.y, 1.0));

	vec3 sunDir = normalize(atmosphere.suns[0].dir);
	vec3 sun = SunWithBloom(rayDir, sunDir);
	sun = smoothstep(0.002, 1.0, sun);
	sun *= Transmittance(cameraPos, sunDir);

	vec3 lum = vec3(0);
	//vec3 lum = RaymarchScattering(cameraPos, rayDir, sunDir, 35);

	lum += sun;

	lum *= 20.0;
	lum /= (smoothstep(0.0, 0.2, clamp(sunDir.y, 0.0, 1.0))*2.0 + 0.15);

	lum = jodieReinhardTonemap(lum);
	lum = pow(lum, vec3(1.0/2.2));
	fragOut = vec4(lum, 1.0);
}