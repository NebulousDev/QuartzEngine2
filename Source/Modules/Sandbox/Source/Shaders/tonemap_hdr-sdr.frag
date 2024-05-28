#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 fragOut;

//layout(std140, set = 0, binding = 0) uniform TonemapUBO
//{
//	float exposure;
//}
//tonemap;

layout(set = 0, binding = 0) uniform sampler2D colorTexture;

vec3 aces(vec3 color)
{
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;

  return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

vec3 tonemap_aces(vec3 color, float exposure, vec3 whitePoint)
{
    vec3 acesColor = aces(color * exposure);
    vec3 whiteScale = vec3(1.0) / aces(whitePoint);
    return acesColor * whiteScale;
}

vec3 uncharted2_tonemap_partial(vec3 color)
{
    float a = 0.15f;
    float b = 0.50f;
    float c = 0.10f;
    float d = 0.20f;
    float e = 0.02f;
    float f = 0.30f;

    return clamp(((color * (a * color + c * b) + d * e) / ( color * (a * color + b) + d * f)) - e / f, 0.0, 1.0);
}

vec3 uncharted2_filmic(vec3 color, float exposure, vec3 whitePoint)
{
    vec3 acesColor = aces(color * exposure);
    vec3 whiteScale = vec3(1.0) / aces(whitePoint);
    return acesColor * whiteScale;
}

void main()
{
    float exposure = 2.0;
    vec3 whitePoint = vec3(11.2f);

    vec3 color = texture(colorTexture, inUV).rgb;
    color = uncharted2_filmic(color, exposure, whitePoint);
    
	fragOut = vec4(color, 1.0);
}