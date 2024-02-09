#pragma once

#include "../SPIRV/Spirv.h"

namespace Quartz
{
	struct VulkanDevice;

	struct VulkanShader
	{
		VkShaderModule			vkShader;
		VulkanDevice*			pDevice;
		String					name;
		VkShaderStageFlagBits	vkStage;
		String					entryPoint;
		Array<SpirvUniform>		uniforms;
		Array<SpirvAttribute>	attributes;
	};
}