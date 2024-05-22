#include "Vulkan/VulkanShaderCache.h"

#include "Log.h"
#include "Engine.h"
#include "Resource/Assets/Shader.h"
#include <iostream>
#include <fstream>

namespace Quartz
{
	VulkanShaderCache::VulkanShaderCache() :
		mpResourceManager(nullptr), mpDevice(nullptr)
	{
		// Nothing
	}

	VulkanShaderCache::VulkanShaderCache(VulkanDevice* pDevice, VulkanResourceManager* pResources) :
		mpResourceManager(pResources), mpDevice(pDevice)
	{
		// Nothing
	}

	VulkanShader* VulkanShaderCache::FindOrCreateShader(const String& filepath)
	{
		auto& it = mShaderMap.Find(filepath);

		if (it != mShaderMap.End())
		{
			return it->value;
		}

		Shader* pShader = Engine::GetAssetManager().GetOrLoadAsset<Shader>(filepath);

		if (!pShader)
		{
			return nullptr;
		}

		ShaderCode* pSpirvCode = nullptr;

		for (ShaderCode& code : pShader->shaderCodes)
		{
			if (code.lang == SHADER_LANG_GLSL_SPIRV)
			{
				pSpirvCode = &code;
			}
		}

		if (!pSpirvCode)
		{
			return nullptr;
		}

		VulkanShader* pVulkanShader = mpResourceManager->CreateShader(mpDevice, filepath, *pSpirvCode->pSourceBuffer);

		mShaderMap.Put(filepath, pVulkanShader);

		return pVulkanShader;
	}
}
