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

		VulkanImage*			mpSkyTransmittanceLUT[3];
		VulkanImage*			mpSkyScatterLUT[3];
		VulkanImage*			mpSkyViewLUT[3];

		VulkanImageView*		mpSkyTransmittanceLUTView[3];
		VulkanImageView*		mpSkyScatterLUTView[3];
		VulkanImageView*		mpSkyViewLUTView[3];

		VkSemaphore				mSkyTransmittanceLUTcomplete[3];
		VkSemaphore				mSkyScatterLUTcomplete[3];
		VkSemaphore				mSkyViewLUTcomplete[3];

		VulkanGraphicsPipeline* mpSkyTransmittanceLUTPipeline;
		VulkanGraphicsPipeline* mpSkyScatterLUTPipeline;
		VulkanGraphicsPipeline* mpSkyViewLUTPipeline;

		VulkanGraphicsPipeline* mpSkyRenderPipeline;

		VkSampler				mVkLUTSampler;

		VulkanCommandPool*		mpImmediateCommandPool;
		VulkanCommandBuffer*	mImmediateCommandBuffers[3];
		VulkanCommandRecorder	mImmediateRecorders[3];
		VkFence					mImmediateFences[3];

		void CopyAtmospherePerFrameData(const AtmosphereValues& atmosphere, 
			const Vec3f& cameraPos, const Vec3f viewDir, float width, float height);

		void PrepareImageForRender(VulkanCommandRecorder& recorder, VulkanImage* pImage);
		void PrepareImageForSampling(VulkanCommandRecorder& recorder, VulkanImage* pImage);

	public:
		void Initialize(VulkanGraphics& graphics, const AtmosphereValues& atmosphere, const SkyRenderSettings& settings, 
			VulkanShaderCache& shaderCache, VulkanPipelineCache& pipelineCache);

		void Update(CameraComponent& camera, TransformComponent& cameraTransform, uSize frameIdx);

		void RecordTransfers(VulkanCommandRecorder& transferRecorder, uSize frameIdx);
		void RenderLUTs(VulkanCommandRecorder& recorder, uSize frameIdx);
		void PreRender(VulkanCommandRecorder& renderRecorder, uSize frameIdx);
		void RecordDraws(VulkanCommandRecorder& renderRecorder, uSize frameIdx);

		VkSemaphore GetLUTsCompleteSemaphore(uSize frameIdx);
	};
}