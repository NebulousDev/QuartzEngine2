#pragma once

#include "GfxAPI.h"
#include "Math/Math.h"

#include "Entity/World.h"
#include "Vulkan/VulkanGraphics.h"
#include "Vulkan/VulkanBufferCache.h"
#include "Vulkan/VulkanCommandRecorder.h"
#include "Vulkan/VulkanPipelineCache.h"
#include "Vulkan/VulkanShaderCache.h"
#include "Vulkan/VulkanBufferWriter.h"
#include "Vulkan/VulkanRenderable.h"
#include "Vulkan/Primatives/VulkanImage.h"

#include "Graphics/Component/CameraComponent.h"
#include "Graphics/Component/TransformComponent.h"
#include "Graphics/Component/LightComponent.h"

#include "Resource/Assets/Image.h"

#include "Graphics/Renderer.h"

namespace Quartz
{

#pragma pack(push,1)

	struct VulkanPointLight
	{
		Vec4f position;
		Vec3f color;
		float intensity;
	};

	struct VulkanRenderableSceneUBO
	{
		Vec3f cameraPosition;
		uInt32 pointLightsCount;
		VulkanPointLight pointLights[10];
	};

	struct VulkanRenderablePerModelUBO
	{
		Mat4f model;
		Mat4f view;
		Mat4f proj;
	};

#pragma pack(pop)

	class QUARTZ_GRAPHICS_API VulkanSceneRenderer
	{
	private:
		VulkanGraphics* mpGraphics;
		VulkanDevice*	mpDevice;

		VulkanGraphicsPipeline* mpDefaultPipeline;
		VulkanGraphicsPipeline* mpTonemapPipeline;

		Array<VulkanRenderable>	mRenderables;
		Array<VulkanRenderable>	mRenderablesSorted;

		// TEMP
		VkSampler mVkDefaultSampler;
		Map<String, VulkanImageView*> mTextureCache;

	private:
		void GenAndCopyImageMipmapped(const Image* pImage, VulkanImage*& pVulkanImage, uInt32 mipCount);

	public:
		void Initialize(VulkanGraphics& graphics, VulkanDevice& device, VulkanShaderCache& shaderCache, 
			VulkanPipelineCache& pipelineCache, uSize maxInFlightCount);

		void Update(EntityWorld& world, VulkanBufferCache& bufferCache, 
			VulkanShaderCache& shaderCache, VulkanPipelineCache& pipelineCache,
			CameraComponent& camera, TransformComponent& cameraTransform, uSize frameIdx);

		void RecordTransfers(VulkanCommandRecorder& transferRecorder, VulkanBufferCache& bufferCache, uSize frameIdx);
		void RecordDraws(VulkanCommandRecorder& renderRecorder, uSize frameIdx);
	};
}