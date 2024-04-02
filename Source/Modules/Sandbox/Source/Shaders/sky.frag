#version 450
#extension GL_ARB_separate_shader_objects : enable

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
layout(binding = 2) uniform sampler2D skyViewLUT;

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

/* LUT Functions */

float safeacos(const float x) {
    return acos(clamp(x, -1.0, 1.0));
}

vec3 LookupTransmittance(vec3 pos, vec3 sunDir)
{
	float height = length(pos);
    vec3 up = pos / height;
	float sunCosZenithAngle = dot(sunDir, up);
	vec2 uv = vec2(clamp(0.5 + 0.5*sunCosZenithAngle, 0.0, 1.0), 
		max(0.0, min(1.0, (height - groundRadius)/(spaceRadius - groundRadius))));
    return texture(transmittanceLUT, uv).rgb;
}

vec3 LookupSkyView(vec3 cameraPos, vec3 rayDir, vec3 sunDir)
{
	float height = length(cameraPos);
    vec3 up = cameraPos / height;
    
    float horizonAngle = safeacos(sqrt(height * height - groundRadius * groundRadius) / height);
    float altitudeAngle = horizonAngle - acos(dot(rayDir, up)); // Between -PI/2 and PI/2
    float azimuthAngle; // Between 0 and 2*PI
    if (abs(altitudeAngle) > (0.5*PI - .0001)) {
        // Looking nearly straight up or down.
        azimuthAngle = 0.0;
    } else {
        vec3 right = cross(sunDir, up);
        vec3 forward = cross(up, right);
        
        vec3 projectedDir = normalize(rayDir - up*(dot(rayDir, up)));
        float sinTheta = dot(projectedDir, right);
        float cosTheta = dot(projectedDir, forward);
        azimuthAngle = atan(sinTheta, cosTheta) + PI;
    }
    
    // Non-linear mapping of altitude angle. See Section 5.3 of the paper.
    float v = 0.5 + 0.5*sign(altitudeAngle)*sqrt(abs(altitudeAngle)*2.0/PI);
    vec2 uv = vec2(azimuthAngle / (2.0*PI), v);
    //uv *= skyLUTRes;
    //uv /= iChannelResolution[1].xy;
    
    return texture(skyViewLUT, uv).rgb;
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

vec3 jodieReinhardTonemap(vec3 c){
    // From: https://www.shadertoy.com/view/tdSXzD
    float l = dot(c, vec3(0.2126, 0.7152, 0.0722));
    vec3 tc = c / (c + 1.0);
    return mix(c / (l + 1.0), tc, tc);
}

/* Main Functions */

void main()
{
	float aspect = atmosphere.height / atmosphere.width; //atmosphere.width / atmosphere.height;

	vec3 camDir = normalize(-atmosphere.viewDir);
	vec3 camRight = normalize(cross(camDir, vec3(0.0, 1.0, 0.0)));
    vec3 camUp = normalize(cross(camRight, camDir));

	float camFOVWidth = PI/3.5;
    float camWidthScale = 2.0*tan(camFOVWidth/2.0);
    float camHeightScale = camWidthScale*aspect;

	vec3 cameraPos = vec3(0.0, groundRadius, 0.0) + atmosphere.cameraPos / KM;
	vec2 coords = 2.0 * inUV - vec2(1.0);
	vec3 rayDir = -normalize(camDir + camRight * coords.x * camWidthScale + camUp * coords.y * camHeightScale);

	vec3 sunDir = normalize(-atmosphere.suns[0].dir);
	vec3 sun = SunWithBloom(rayDir, sunDir);
	sun = smoothstep(0.002, 1.0, sun);

	if(length(sun) > 0.0)
	{
		if(RaySphere(cameraPos, rayDir, groundCenter, groundRadius) >= 0.0)
		{
			sun *= 0;
		}
		else
		{
			sun *= LookupTransmittance(cameraPos, sunDir);
		}
	}

	vec3 lum = LookupSkyView(cameraPos, rayDir, sunDir);

	//lum += sun;

	lum *= 20.0;
	lum /= (smoothstep(0.0, 0.2, clamp(sunDir.y, 0.0, 1.0))*2.0 + 0.15);

	lum = jodieReinhardTonemap(lum);
	lum = pow(lum, vec3(1.0/2.2));
	fragOut = vec4(lum, 1.0);
}