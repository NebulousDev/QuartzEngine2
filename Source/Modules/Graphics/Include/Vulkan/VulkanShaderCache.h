#pragma once

#include "../GfxAPI.h"
#include "VulkanResourceManager.h"
#include "Primatives/VulkanShader.h"

namespace Quartz
{
	class QUARTZ_GRAPHICS_API VulkanShaderCache
	{
	private:
		VulkanResourceManager*		mpResourceManager;
		VulkanDevice*				mpDevice;
		Map<String, VulkanShader*>	mShaderMap;

	public:
		VulkanShaderCache();
		VulkanShaderCache(VulkanDevice* pDevice, VulkanResourceManager* pResources);

		VulkanShader* FindOrCreateShader(const String& filepath, VkShaderStageFlagBits vkShaderStage);
	};
}