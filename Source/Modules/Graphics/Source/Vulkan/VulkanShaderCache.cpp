#include "Vulkan/VulkanShaderCache.h"

#include "Log.h"
#include "shaderc/shaderc.hpp"
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

	// @TODO: Temp location, should be moved to Engine Module
	bool ReadFile(const String& filepath, Array<char>& outData)
	{
		std::ifstream file(filepath.Str());

		if (!file.is_open())
		{
			LogError("ReadFile failed: Cannot open file '%s'", filepath.Str());
			return false;
		}

		file.seekg(0, file.end);
		uSize fileSize = file.tellg();
		outData.Resize(fileSize + 1);

		file.seekg(0, file.beg);
		file.read(outData.Data(), fileSize);
		outData[fileSize] = 0;

		file.close();

		return true;
	}

	bool CompileShader(const String& filepath, const char* source, Array<uInt8>& outSpirv, shaderc_shader_kind kind)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		LogInfo("Compiling Shader '%s'...", filepath.Str());

		options.SetOptimizationLevel(shaderc_optimization_level_performance);

		shaderc::SpvCompilationResult module =
			compiler.CompileGlslToSpv(source, kind, "shaderc-shader", options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LogError("Failed compiling shader '%s':\n%s", filepath.Str(), module.GetErrorMessage().c_str());
			return false;
		}

		std::vector<uInt32> binary = { module.cbegin(), module.cend() };
		outSpirv.Resize(binary.size() * sizeof(unsigned int));
		memcpy_s(outSpirv.Data(), outSpirv.Size(), binary.data(), binary.size() * sizeof(unsigned int));

		return true;
	}

	shaderc_shader_kind GetShadercShaderKind(VkShaderStageFlagBits vkShaderStage)
	{
		switch (vkShaderStage)
		{
			case VK_SHADER_STAGE_VERTEX_BIT:					return shaderc_vertex_shader;
			case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:		return shaderc_tess_control_shader;
			case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:	return shaderc_tess_evaluation_shader;
			case VK_SHADER_STAGE_GEOMETRY_BIT:					return shaderc_geometry_shader;
			case VK_SHADER_STAGE_FRAGMENT_BIT:					return shaderc_fragment_shader;
			case VK_SHADER_STAGE_COMPUTE_BIT:					return shaderc_compute_shader;
			case VK_SHADER_STAGE_RAYGEN_BIT_KHR:				return shaderc_raygen_shader;
			case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:				return shaderc_anyhit_shader;
			case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:			return shaderc_closesthit_shader;
			case VK_SHADER_STAGE_MISS_BIT_KHR:					return shaderc_miss_shader;
			case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:			return shaderc_intersection_shader;
			case VK_SHADER_STAGE_CALLABLE_BIT_KHR:				return shaderc_callable_shader;
			case VK_SHADER_STAGE_TASK_BIT_EXT:					return shaderc_task_shader;
			case VK_SHADER_STAGE_MESH_BIT_EXT:					return shaderc_mesh_shader;
		}

		return shaderc_glsl_infer_from_source;
	}

	VulkanShader* VulkanShaderCache::FindOrCreateShader(const String& filepath, VkShaderStageFlagBits vkShaderStage)
	{
		auto& it = mShaderMap.Find(filepath);

		if (it != mShaderMap.End())
		{
			return it->value;
		}

		Array<char> shaderData;

		if (!ReadFile(filepath, shaderData))
		{
			LogError("FindOrCreateShader failed: Could not read file '%s'", filepath.Str());
			return nullptr;
		}

		Array<uInt8> spirvData;
		if (!CompileShader(filepath, (const char*)shaderData.Data(), spirvData, GetShadercShaderKind(vkShaderStage)))
		{
			LogError("FindOrCreateShader failed: Shader compilation failed for '%s'", filepath.Str());
			return nullptr;
		}

		VulkanShader* pShader = mpResourceManager->CreateShader(mpDevice, filepath, spirvData);

		mShaderMap.Put(filepath, pShader);

		return pShader;
	}
}
