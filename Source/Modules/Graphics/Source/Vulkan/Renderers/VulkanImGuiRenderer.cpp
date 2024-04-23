#include "Vulkan/Renderers/VulkanImGuiRenderer.h"

#include "Vulkan/VulkanRenderer.h"
#include "Engine.h"
#include "Log.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

#if QUARTZAPP_GLFW
#include "backends/imgui_impl_glfw.h"
#endif

#if QUARTZAPP_WINAPI
#include "backends/imgui_impl_win32.h"
#endif

namespace Quartz
{
	void VulkanImGuiRenderer::Initialize(VulkanGraphics& graphics, VulkanDevice& device, Window& window, VkPipelineRenderingCreateInfo& renderingInfo)
	{
		mpWindow = &window;
		mWindowApi = window.GetParentApplication()->GetWindowAPI();

		IMGUI_CHECKVERSION();
		ImGuiContext* imguiContext = ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		ImGui::StyleColorsDark();

		switch (mWindowApi)
		{
#if QUARTZAPP_GLFW
			case WINDOW_API_GLFW:
			{
				ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)window.GetNativeHandle(), true);
				break;
			}
#endif
#if QUARTZAPP_WINAPI
			case WINDOW_API_WINAPI:
			{
				ImGui_ImplWin32_Init(window.GetNativeHandle());
				break;
			}
#endif
		default:
			LogError("ImGuiRenderer Initalization Failed: Unsupported window API.");
			return;
		}

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

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags			= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets		= 1000;
		poolInfo.poolSizeCount	= std::size(pool_sizes);
		poolInfo.pPoolSizes		= pool_sizes;

		vkCreateDescriptorPool(device.vkDevice, &poolInfo, VK_NULL_HANDLE, &mvkDescriptorPool);

		ImGui_ImplVulkan_InitInfo imguiInfo = {};
		imguiInfo.Allocator						= VK_NULL_HANDLE;
		imguiInfo.Instance						= graphics.vkInstance;
		imguiInfo.PhysicalDevice				= device.pPhysicalDevice->vkPhysicalDevice;
		imguiInfo.Device						= device.vkDevice;
		imguiInfo.Queue							= device.queues.graphics;
		imguiInfo.DescriptorPool				= mvkDescriptorPool;
		imguiInfo.MinImageCount					= 3;
		imguiInfo.ImageCount					= 3;
		imguiInfo.MSAASamples					= VK_SAMPLE_COUNT_1_BIT;
		imguiInfo.UseDynamicRendering			= true;
		imguiInfo.PipelineRenderingCreateInfo	= renderingInfo;

		if (!ImGui_ImplVulkan_Init(&imguiInfo))
		{
			LogError("ImGuiRenderer Initalization Failed: Unsupported window API.");
			vkDestroyDescriptorPool(device.vkDevice, mvkDescriptorPool, VK_NULL_HANDLE);
			return;
		}

		ImGui_ImplVulkan_CreateFontsTexture();
	}

	void BuildSettingsWindow(VulkanRenderer* pRenderer)
	{
		ImGui::SetNextWindowPos({ 0,0 });
		ImGui::SetNextWindowSize({150, 100});

		bool debugOpen = false;
		ImGui::Begin("DebugInfo", &debugOpen, 
			ImGuiWindowFlags_NoInputs | 
			ImGuiWindowFlags_NoMove | 
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoTitleBar);

		const double fps = pRenderer->GetAverageFPS();
		const double ups = Engine::GetRuntime().GetAverageUps();
		const double tps = Engine::GetRuntime().GetAverageTps();

		ImGui::Text("FPS: %.2lf", fps);
		ImGui::Text("UPS: %.2lf", ups);
		ImGui::Text("TPS: %.2lf", tps);

		ImGui::End();
	}

	double accumTime = 0.05;
	double updateTime = 0.05;

	void VulkanImGuiRenderer::Update(VulkanRenderer* pRenderer, double deltaTime)
	{
		accumTime += deltaTime;
		if (accumTime < updateTime)
		{
			return; // Dont update
		}

		accumTime = 0.0;

		ImGui_ImplVulkan_NewFrame();
		
		switch (mWindowApi)
		{
#if QUARTZAPP_GLFW
			case WINDOW_API_GLFW:
			{
				ImGui_ImplGlfw_NewFrame();
				break;
			}
#endif
#if QUARTZAPP_WINAPI
			case WINDOW_API_WINAPI:
			{
				ImGui_ImplWin32_NewFrame();
				break;
			}
#endif
		}

		ImGui::NewFrame();
		BuildSettingsWindow(pRenderer);
		ImGui::Render();
	}

	void VulkanImGuiRenderer::RecordDraws(VulkanCommandRecorder& renderRecorder)
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderRecorder.GetCommandBuffer().vkCommandBuffer);
	}
}