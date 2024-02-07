#pragma once

#include "SPIRV/Spirv.h"

namespace Quartz
{
	struct VulkanShader
	{
		String					name;
		VkShaderModule			vkShader;
		VkShaderStageFlagBits	vkStage;
		String					entryPoint;
		Array<SpirvUniform>		uniforms;
		Array<SpirvAttribute>	attributes;
	};
}