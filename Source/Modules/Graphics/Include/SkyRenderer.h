#pragma once

#include "GfxAPI.h"
#include "Math/Math.h"
#include "Vulkan/VulkanGraphics.h"
#include "Vulkan/VulkanCommandRecorder.h"
#include "Vulkan/VulkanPipelineCache.h"
#include "Vulkan/VulkanShaderCache.h"

namespace Quartz
{
	struct AtmosphereProperties
	{
		Vec3f rayleighScattering;
		float rayleighAbsorbtion;
		float mieScattering;
		float mieAbsorbtion;
		float ozoneScattering;
		Vec3f ozoneAbsorbtion;
	};

	class QUARTZ_GRAPHICS_API VulkanSkyRenderer
	{
	private:
		AtmosphereProperties	mAtmosphere;
		float					mTimeOfDay;

		VulkanBuffer*			mpTriVertexBuffer;
		VulkanGraphicsPipeline* mpSkyRenderPipeline;

	public:
		void Initialize(VulkanGraphics& graphics, const AtmosphereProperties& atmosphere, VulkanShaderCache& shaderCache, VulkanPipelineCache& pipelineCache);

		void RecordTransfers(VulkanCommandRecorder& transferRecorder);
		void RecordDraws(VulkanCommandRecorder& renderRecorder);
	};
}