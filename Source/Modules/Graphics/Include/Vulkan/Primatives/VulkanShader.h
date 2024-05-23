#pragma once

#include "Resource/SPIRV/Spirv.h"

namespace Quartz
{
	struct VulkanDevice;
	struct Shader;

	struct VulkanShader
	{
		VkShaderModule			vkShader;
		Shader*					pShaderAsset;
		VulkanDevice*			pDevice;
		String					name;
		VkShaderStageFlagBits	vkStage;
		String					entryPoint;
		Array<SpirvUniform>		uniforms;
		Array<SpirvAttribute>	attributes;
	};
}