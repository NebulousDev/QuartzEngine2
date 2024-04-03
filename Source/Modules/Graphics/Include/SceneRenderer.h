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

#include "Component/CameraComponent.h"
#include "Component/TransformComponent.h"

namespace Quartz
{
	struct VulkanRenderablePerModelUBO
	{
		Mat4f model;
		Mat4f view;
		Mat4f proj;
	};

	class QUARTZ_GRAPHICS_API VulkanSceneRenderer
	{
	private:
		VulkanGraphicsPipeline* mpDefaultPipeline;

		Array<VulkanRenderable>	mRenderables;
		Array<VulkanRenderable>	mRenderablesSorted;

	public:
		void Initialize(VulkanGraphics& graphics, VulkanShaderCache& shaderCache, 
			VulkanPipelineCache& pipelineCache, uSize maxInFlightCount);

		void Update(EntityWorld& world, VulkanBufferCache& bufferCache, 
			VulkanShaderCache& shaderCache, VulkanPipelineCache& pipelineCache,
			CameraComponent& camera, TransformComponent& cameraTransform, uSize frameIdx);

		void RecordTransfers(VulkanCommandRecorder& transferRecorder, VulkanBufferCache& bufferCache, uSize frameIdx);
		void RecordDraws(VulkanCommandRecorder& renderRecorder, uSize frameIdx);
	};
}