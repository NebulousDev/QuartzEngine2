#pragma once

#include "GfxAPI.h"
#include "Math/Math.h"

#include "Vulkan/VulkanGraphics.h"
#include "Vulkan/VulkanCommandRecorder.h"
#include "Vulkan/VulkanPipelineCache.h"
#include "Vulkan/VulkanShaderCache.h"
#include "Vulkan/VulkanBufferWriter.h"

#include "Component/CameraComponent.h"
#include "Component/TransformComponent.h"

namespace Quartz
{
	#pragma pack(push,1)
	struct AtmosphereSun
	{
		Vec3f sunDir;
		float sunIntensity;
	};
	#pragma pack(pop)

	struct AtmosphereValues
	{
		Vec3f rayleighScattering;
		float rayleighAbsorbtion;
		float mieScattering;
		float mieAbsorbtion;
		float ozoneScattering;
		Vec3f ozoneAbsorbtion;
		AtmosphereSun suns[2];
	};

	struct SkyRenderSettings
	{
		Vec2u transmittanceLUTSize;
		Vec2u scatterLUTSize;
		Vec2u viewLUTSize;
	};

	class QUARTZ_GRAPHICS_API VulkanSkyRenderer
	{
	private:
		#pragma pack(push,1)
		struct AtmospherePerFrameData
		{
			Vec3f rayleighScattering;
			float rayleighAbsorbtion;
			float mieScattering;
			float mieAbsorbtion;
			float ozoneScattering;
			float _pad0_;
			Vec3f ozoneAbsorbtion;
			float _pad1_;
			AtmosphereSun suns[2];

			// TEMP:
			Vec3f cameraPos;
			float _pad2_;
			Vec3f viewDir;
			float width;
			float height;
		};
		#pragma pack(pop)

	private:
		VulkanGraphics*			mpGraphics;

		AtmosphereValues		mAtmosphere;
		SkyRenderSettings		mSettings;

		VulkanBuffer*			mpSkyPerFrameBuffer;
		VulkanBuffer*			mpSkyPerFrameStagingBuffer;
		VulkanBufferWriter		mpSkyPerFrameWriter;
		AtmospherePerFrameData*	mpSkyPerFrameData;

		VulkanImage*			mpSkyTransmittanceLUT[VULKAN_GRAPHICS_MAX_IN_FLIGHT];
		VulkanImage*			mpSkyScatterLUT[VULKAN_GRAPHICS_MAX_IN_FLIGHT];
		VulkanImage*			mpSkyViewLUT[VULKAN_GRAPHICS_MAX_IN_FLIGHT];

		VulkanImageView*		mpSkyTransmittanceLUTView[VULKAN_GRAPHICS_MAX_IN_FLIGHT];
		VulkanImageView*		mpSkyScatterLUTView[VULKAN_GRAPHICS_MAX_IN_FLIGHT];
		VulkanImageView*		mpSkyViewLUTView[VULKAN_GRAPHICS_MAX_IN_FLIGHT];

		VkSemaphore				mSkyTransmittanceLUTcomplete[VULKAN_GRAPHICS_MAX_IN_FLIGHT];
		VkSemaphore				mSkyScatterLUTcomplete[VULKAN_GRAPHICS_MAX_IN_FLIGHT];
		VkSemaphore				mSkyViewLUTcomplete[VULKAN_GRAPHICS_MAX_IN_FLIGHT];

		VulkanGraphicsPipeline* mpSkyTransmittanceLUTPipeline;
		VulkanGraphicsPipeline* mpSkyScatterLUTPipeline;
		VulkanGraphicsPipeline* mpSkyViewLUTPipeline;

		VulkanGraphicsPipeline* mpSkyRenderPipeline;

		VkSampler				mVkLUTSampler;

		void CopyAtmospherePerFrameData(const AtmosphereValues& atmosphere, 
			const Vec3f& cameraPos, const Vec3f viewDir, float width, float height);

		void PrepareImageForRender(VulkanCommandRecorder& recorder, VulkanImage* pImage);
		void PrepareImageForSampling(VulkanCommandRecorder& recorder, VulkanImage* pImage);

	public:
		void Initialize(VulkanGraphics& graphics, const AtmosphereValues& atmosphere, const SkyRenderSettings& settings, 
			VulkanShaderCache& shaderCache, VulkanPipelineCache& pipelineCache, uSize maxInFlightCount);

		void Update(CameraComponent& camera, TransformComponent& cameraTransform, uSize frameIdx);

		void RecordTransfers(VulkanCommandRecorder& transferRecorder, uSize frameIdx);
		void RenderLUTs(VulkanCommandRecorder& recorder, uSize frameIdx);
		void RecordPreDraws(VulkanCommandRecorder& renderRecorder, uSize frameIdx);
		void RecordDraws(VulkanCommandRecorder& renderRecorder, uSize frameIdx);

		VkSemaphore GetLUTsCompleteSemaphore(uSize frameIdx);
	};
}