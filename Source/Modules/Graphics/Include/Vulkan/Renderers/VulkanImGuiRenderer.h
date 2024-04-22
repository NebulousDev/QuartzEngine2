#pragma once

#include "GfxAPI.h"

#include "Window.h"
#include "Application.h"

#include "Vulkan/VulkanGraphics.h"
#include "Vulkan/VulkanCommandRecorder.h"

namespace Quartz
{
	class QUARTZ_GRAPHICS_API VulkanImGuiRenderer
	{
	private:
		Window*				mpWindow;
		WindowAPI			mWindowApi;
		VkDescriptorPool	mvkDescriptorPool;

	public:
		void Initialize(VulkanGraphics& graphics, VulkanDevice& device, Window& window, VkPipelineRenderingCreateInfo& renderingInfo);

		void Update();

		void RecordDraws(VulkanCommandRecorder& renderRecorder);
	};
}