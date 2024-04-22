#pragma once

#include "GfxAPI.h"
#include "Vulkan/VulkanGraphics.h"
#include "Vulkan/VulkanCommandRecorder.h"
#include "Vulkan/Primatives/VulkanSwapchain.h"

namespace Quartz
{
	class QUARTZ_GRAPHICS_API VulkanImGuiRenderer
	{

	public:
		void Initialize(VulkanGraphics& graphics, void* pWindowHandle, VulkanSwapchain& swapchain);

		void Update();

		void RecordDraws(VulkanCommandRecorder& renderRecorder);
	};
}