#include "ImGuiRenderer.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

#include "R:\Quartz 2.0\QuartzEngine2\Source\Modules\Platform\Include\Platform.h"

namespace Quartz
{
	void VulkanImGuiRenderer::Initialize(VulkanGraphics& graphics, void* pWindowHandle, VulkanSwapchain& swapchain)
	{
		VulkanDevice& device = *graphics.pPrimaryDevice;

		//PlatformSingleton& platform = Engine::GetWorld().Get<PlatformSingleton>();
		//platform.pApplication.

		IMGUI_CHECKVERSION();
		ImGuiContext* imguiContext = ImGui::CreateContext();

		//ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)pWindowHandle, true);

		//ImGui::SetCurrentContext(imguiContext);

		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags			= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets		= 1000;
		pool_info.poolSizeCount = std::size(pool_sizes);
		pool_info.pPoolSizes	= pool_sizes;

		VkDescriptorPool imguiPool;
		vkCreateDescriptorPool(device.vkDevice, &pool_info, nullptr, &imguiPool);

		VkPipelineRenderingCreateInfo renderingInfo = {};
		renderingInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingInfo.colorAttachmentCount		= 1;
		renderingInfo.pColorAttachmentFormats	= &swapchain.images[0]->vkFormat;
		renderingInfo.depthAttachmentFormat		= VK_FORMAT_D24_UNORM_S8_UINT; // Temp

		ImGui_ImplVulkan_InitInfo imguiInfo = {};
		imguiInfo.Allocator						= VK_NULL_HANDLE;
		imguiInfo.Instance						= graphics.vkInstance;
		imguiInfo.PhysicalDevice				= device.pPhysicalDevice->vkPhysicalDevice;
		imguiInfo.Device						= device.vkDevice;
		imguiInfo.Queue							= device.queues.graphics;
		imguiInfo.DescriptorPool				= imguiPool;
		imguiInfo.MinImageCount					= 3;
		imguiInfo.ImageCount					= 3;
		imguiInfo.MSAASamples					= VK_SAMPLE_COUNT_1_BIT;
		imguiInfo.UseDynamicRendering			= true;
		imguiInfo.PipelineRenderingCreateInfo	= renderingInfo;

		ImGui_ImplVulkan_Init(&imguiInfo);

		ImGui_ImplVulkan_CreateFontsTexture();
	}

	void VulkanImGuiRenderer::Update()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow();
		ImGui::Render();
	}

	void VulkanImGuiRenderer::RecordDraws(VulkanCommandRecorder& renderRecorder)
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderRecorder.GetCommandBuffer().vkCommandBuffer);
	}
}