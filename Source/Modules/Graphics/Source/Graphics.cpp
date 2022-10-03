#include "System/System.h"

#include "Quartz.h"
#include "Entity/World.h"
#include "Runtime/Runtime.h"
#include "Log.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	struct VulkanPhysicalDevice
	{
		VkPhysicalDevice					vkPhysicalDevice;
		VkPhysicalDeviceProperties			vkProperties;
		VkPhysicalDeviceFeatures			vkFeatures;
		VkPhysicalDeviceMemoryProperties	vkMemoryProperties;
		Array<VkQueueFamilyProperties>		vkQueueFamilyProperties;

		struct
		{
			uInt32							graphics;
			uInt32							compute;
			uInt32							transfer;
			uInt32							present;
		}
		primaryQueueFamilyIndices;
	};

	struct VulkanDevice
	{
		VkDevice				vkDevice;
		VulkanPhysicalDevice*	pPhysicalDevice;

		struct
		{
			VkQueue				graphics;
			VkQueue				compute;
			VkQueue				transfer;
			VkQueue				present;
		}
		queues;
	};

	struct VulkanRenderContext
	{
		EntityWorld* pWorld;
		VkSurfaceKHR vkSurface;
		VkSemaphore vkImageAvailableSemaphore;
		VkSemaphore vkImageFinishedSemaphore;
	};

	struct VulkanData
	{
		VkInstance					vkInstance;
		VkApplicationInfo			vkAppInfo;
		Array<const char*>			enabledExtensions;
		VulkanPhysicalDevice		primaryPhysicalDevice;
		Array<VulkanPhysicalDevice> physicalDevices;
		VulkanDevice				primaryDevice;
		Array<VulkanDevice>			devices;
		bool						ready;

		VulkanRenderContext			renderContext;
	};

	struct VulkanReady
	{
		VulkanData* pVulkanData;
	};

	EntityWorld*	gpEntityWorld;
	Runtime*		gpRuntime;
	VulkanData*		gpVulkanData;

	const char* VendorNameFromID(uInt32 vendorID)
	{
		switch (vendorID)
		{
			case 0x1002: return "AMD";
			case 0x1010: return "ImgTec";
			case 0x10DE: return "NVIDIA";
			case 0x13B5: return "ARM";
			case 0x5143: return "Qualcomm";
			case 0x8086: return "INTEL";

			default: return "Unknown";
		}
	}

	const char* TypeNameFromVkPhysicalDeviceType(VkPhysicalDeviceType type)
	{
		switch (type)
		{
			case VK_PHYSICAL_DEVICE_TYPE_OTHER:				return "Other";
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:	return "Integrated";
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:		return "Discrete";
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:		return "Virtual";
			case VK_PHYSICAL_DEVICE_TYPE_CPU:				return "CPU";
			default:										return "Unknown";
		}
	}

	void PrintVulkanData(VulkanData* pData)
	{
		LogRaw("Vulkan Data:\n");

		LogRaw("> Instance: [%p]\n", pData->vkInstance);

		LogRaw("> Physical devices found:\n");
		for (const VulkanPhysicalDevice& physicalDevice : pData->physicalDevices)
		{
			char driverVersionString[64] = {};

			// https://github.com/SaschaWillems/vulkan.gpuinfo.org/blob/1e6ca6e3c0763daabd6a101b860ab4354a07f5d3/functions.php#L294

			if (physicalDevice.vkProperties.vendorID == 0x10DE) // NVIDIA
			{
				sprintf_s(driverVersionString, "%d.%d.%d.%d", 
					(physicalDevice.vkProperties.driverVersion >> 22) & 0x3FF,
					(physicalDevice.vkProperties.driverVersion >> 14) & 0x0FF,
					(physicalDevice.vkProperties.driverVersion >> 6)  & 0x0FF,
					(physicalDevice.vkProperties.driverVersion >> 0)  & 0x003);
			}
			else if (physicalDevice.vkProperties.vendorID == 0x1002) // AMD
			{
				sprintf_s(driverVersionString, "%d.%d.%d",
					(physicalDevice.vkProperties.driverVersion >> 22),
					(physicalDevice.vkProperties.driverVersion >> 12) & 0x3FF,
					(physicalDevice.vkProperties.driverVersion >> 0) & 0xFFF);
			}
			else if (physicalDevice.vkProperties.vendorID == 0x8086) // INTEL
			{
				sprintf_s(driverVersionString, "%d.%d",
					physicalDevice.vkProperties.driverVersion >> 14,
					physicalDevice.vkProperties.driverVersion & 0x0FF);
			}
			else
			{
				// Format is not known
				sprintf_s(driverVersionString, "%u", physicalDevice.vkProperties.driverVersion);
			}

			LogRaw("  [%p]%s:\n   Name: %s\n   Vendor: %s (%X)\n   Type: %s\n   Driver: %s\n   VRAM: %lu MB\n",
				physicalDevice.vkPhysicalDevice,
				physicalDevice.vkPhysicalDevice == pData->primaryPhysicalDevice.vkPhysicalDevice ? "(Primary)" : "",
				physicalDevice.vkProperties.deviceName,
				VendorNameFromID(physicalDevice.vkProperties.vendorID),
				physicalDevice.vkProperties.vendorID,
				TypeNameFromVkPhysicalDeviceType(physicalDevice.vkProperties.deviceType),
				driverVersionString,
				physicalDevice.vkMemoryProperties.memoryHeaps[0].size / 1048576UL
			);
		}

		LogRaw("> Extensions loaded:\n");
		for (const char* ext : pData->enabledExtensions)
		{
			LogRaw("  %s\n", ext);
		}
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT		messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT				messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		{
			LogTrace("Vulkan: %s", pCallbackData->pMessage);
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			LogInfo("Vulkan: %s", pCallbackData->pMessage);
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			LogWarning("Vulkan: %s", pCallbackData->pMessage);
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			LogError("Vulkan: %s", pCallbackData->pMessage);
		}

		return VK_FALSE;
	}

	bool CreateVulkanInstance(VulkanData* pData)
	{
		VkInstance vkInstance;

		VkApplicationInfo vkAppInfo = {};
		vkAppInfo.apiVersion		= VK_VERSION_1_2;
		vkAppInfo.pEngineName		= "Quartz Engine 2";
		vkAppInfo.pApplicationName	= "Quartz Sandbox";

		Array<const char*> extentions =
		{
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
			VK_KHR_SURFACE_EXTENSION_NAME,
			"VK_KHR_win32_surface"
		};

		VkDebugUtilsMessengerCreateInfoEXT vkDebugMessengerInfo = {};
		vkDebugMessengerInfo.sType				= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		vkDebugMessengerInfo.messageSeverity	= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
												| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
												| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		vkDebugMessengerInfo.messageType		= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
												| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
												| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		vkDebugMessengerInfo.pfnUserCallback	= DebugCallback;
		vkDebugMessengerInfo.pUserData			= NULL;
		vkDebugMessengerInfo.pNext				= NULL;

		VkInstanceCreateInfo vkInstanceInfo		= {};
		vkInstanceInfo.pApplicationInfo			= &vkAppInfo;
		vkInstanceInfo.enabledExtensionCount	= extentions.Size();
		vkInstanceInfo.ppEnabledExtensionNames	= extentions.Data();
		vkInstanceInfo.enabledLayerCount		= 0;
		vkInstanceInfo.pNext					= (VkDebugUtilsMessengerCreateInfoEXT*)&vkDebugMessengerInfo;

		VkResult result = vkCreateInstance(&vkInstanceInfo, nullptr, &vkInstance);

		if (result == VK_SUCCESS)
		{
			pData->vkInstance			= vkInstance;
			pData->vkAppInfo			= vkAppInfo;
			pData->enabledExtensions	= extentions;

			return true;
		}
		else
		{
			LogFatal("Failed to create Vulkan instance: vkCreateInstance failed. See Vulkan logs for details.");
			return false;
		}
	}

	bool QueryVulkanPhysicalDevices(VulkanData* pData)
	{
		uSize physicalDeviceCount;
		if (vkEnumeratePhysicalDevices(pData->vkInstance, &physicalDeviceCount, VK_NULL_HANDLE) != VK_SUCCESS)
		{
			LogFatal("Failed to enumerate Vulkan physical devices: vkEnumeratePhysicalDevices failed. See Vulkan logs for details.");
			return false;
		}

		if (physicalDeviceCount > 0)
		{
			Array<VkPhysicalDevice> vkPhysicalDeviceList(physicalDeviceCount);
			vkEnumeratePhysicalDevices(pData->vkInstance, &physicalDeviceCount, vkPhysicalDeviceList.Data());

			for (VkPhysicalDevice vkPhysicalDevice : vkPhysicalDeviceList)
			{
				VulkanPhysicalDevice vulkanPhysicalDevice = {};
				vulkanPhysicalDevice.vkPhysicalDevice = vkPhysicalDevice;

				vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vulkanPhysicalDevice.vkProperties);
				vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &vulkanPhysicalDevice.vkFeatures);
				vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &vulkanPhysicalDevice.vkMemoryProperties);

				uSize queueFamilyCount = 0;
				vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, VK_NULL_HANDLE);

				if (queueFamilyCount > 0)
				{
					vulkanPhysicalDevice.vkQueueFamilyProperties.Resize(queueFamilyCount);
					vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, vulkanPhysicalDevice.vkQueueFamilyProperties.Data());

					uInt32 graphicsIndex = -1;
					uInt32 computeIndex  = -1;
					uInt32 transferIndex = -1;
					uInt32 presentIndex  = -1;

					for (uSize i = 0; i < queueFamilyCount; i++)
					{
						VkQueueFamilyProperties& vkProperties = vulkanPhysicalDevice.vkQueueFamilyProperties[i];

						if (vkProperties.queueCount > 0)
						{
							if ((graphicsIndex == -1) 
								&& (vkProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT))
							{
								graphicsIndex = i;
							}

							if ((computeIndex == -1 || computeIndex == graphicsIndex) 
								&& (vkProperties.queueFlags & VK_QUEUE_COMPUTE_BIT))
							{
								computeIndex = i;
							}

							if ((transferIndex == -1 || transferIndex == graphicsIndex || transferIndex == computeIndex) 
								&& (vkProperties.queueFlags & VK_QUEUE_TRANSFER_BIT))
							{
								transferIndex = i;
							}
						}
					}

					// Currently no way to check presentation capibilites
					presentIndex = graphicsIndex;

					vulkanPhysicalDevice.primaryQueueFamilyIndices.graphics = graphicsIndex;
					vulkanPhysicalDevice.primaryQueueFamilyIndices.compute  = computeIndex;
					vulkanPhysicalDevice.primaryQueueFamilyIndices.transfer = transferIndex;
					vulkanPhysicalDevice.primaryQueueFamilyIndices.present  = presentIndex;
				}

				pData->physicalDevices.PushBack(vulkanPhysicalDevice);
			}

			return true;
		}
		else
		{
			LogFatal("Failed to enumerate Vulkan physical devices: No valid devices found.");
			return false;
		}
	}

	bool CreateVulkanDevices(VulkanData* pData)
	{
		float queuePriorities[1] = { 1.0f };

		Array<const char*> deviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		for (VulkanPhysicalDevice& physicalDevice : pData->physicalDevices)
		{
			Array<VkDeviceQueueCreateInfo> queueCreateInfos;

			if (physicalDevice.primaryQueueFamilyIndices.graphics != -1)
			{
				VkDeviceQueueCreateInfo queueInfo = {};
				queueInfo.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex	= physicalDevice.primaryQueueFamilyIndices.graphics;
				queueInfo.queueCount		= 1;
				queueInfo.pQueuePriorities	= queuePriorities;

				queueCreateInfos.PushBack(queueInfo);
			}

			if (physicalDevice.primaryQueueFamilyIndices.compute != -1 
				&& physicalDevice.primaryQueueFamilyIndices.compute != physicalDevice.primaryQueueFamilyIndices.graphics)
			{
				VkDeviceQueueCreateInfo queueInfo = {};
				queueInfo.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex	= physicalDevice.primaryQueueFamilyIndices.compute;
				queueInfo.queueCount		= 1;
				queueInfo.pQueuePriorities	= queuePriorities;

				queueCreateInfos.PushBack(queueInfo);
			}

			if (physicalDevice.primaryQueueFamilyIndices.transfer != -1
				&& physicalDevice.primaryQueueFamilyIndices.transfer != physicalDevice.primaryQueueFamilyIndices.graphics
				&& physicalDevice.primaryQueueFamilyIndices.transfer != physicalDevice.primaryQueueFamilyIndices.compute)
			{
				VkDeviceQueueCreateInfo queueInfo = {};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = physicalDevice.primaryQueueFamilyIndices.transfer;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = queuePriorities;

				queueCreateInfos.PushBack(queueInfo);
			}

			if (physicalDevice.primaryQueueFamilyIndices.present != -1
				&& physicalDevice.primaryQueueFamilyIndices.present != physicalDevice.primaryQueueFamilyIndices.graphics
				&& physicalDevice.primaryQueueFamilyIndices.present != physicalDevice.primaryQueueFamilyIndices.compute
				&& physicalDevice.primaryQueueFamilyIndices.present != physicalDevice.primaryQueueFamilyIndices.transfer)
			{
				VkDeviceQueueCreateInfo queueInfo = {};
				queueInfo.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex	= physicalDevice.primaryQueueFamilyIndices.present;
				queueInfo.queueCount		= 1;
				queueInfo.pQueuePriorities	= queuePriorities;

				queueCreateInfos.PushBack(queueInfo);
			}

			VkDeviceCreateInfo deviceInfo		= {};
			deviceInfo.sType					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceInfo.queueCreateInfoCount		= queueCreateInfos.Size();
			deviceInfo.pQueueCreateInfos		= queueCreateInfos.Data();
			deviceInfo.enabledLayerCount		= 0;
			deviceInfo.ppEnabledLayerNames		= nullptr;
			deviceInfo.enabledExtensionCount	= deviceExtensions.Size();
			deviceInfo.ppEnabledExtensionNames	= deviceExtensions.Data();
			deviceInfo.pEnabledFeatures			= &physicalDevice.vkFeatures;

			VulkanDevice vulkanDevice = {};
			
			if (vkCreateDevice(physicalDevice.vkPhysicalDevice, &deviceInfo, nullptr, &vulkanDevice.vkDevice) != VK_SUCCESS)
			{
				LogFatal("Failed to create logical vulkan device: vkCreateDevice failed. See Vulkan logs for details.");
				return false;
			}

			vulkanDevice.pPhysicalDevice = &physicalDevice;

			vkGetDeviceQueue(vulkanDevice.vkDevice, physicalDevice.primaryQueueFamilyIndices.graphics, 0, &vulkanDevice.queues.graphics);
			vkGetDeviceQueue(vulkanDevice.vkDevice, physicalDevice.primaryQueueFamilyIndices.compute, 0, &vulkanDevice.queues.compute);
			vkGetDeviceQueue(vulkanDevice.vkDevice, physicalDevice.primaryQueueFamilyIndices.transfer, 0, &vulkanDevice.queues.transfer);
			vkGetDeviceQueue(vulkanDevice.vkDevice, physicalDevice.primaryQueueFamilyIndices.present, 0, &vulkanDevice.queues.present);

			gpVulkanData->devices.PushBack(vulkanDevice);
		}

		return true;
	}

	bool ChoosePrimaryDevices(VulkanData* pData)
	{
		uSize bestDeviceIndex = -1;
		int32 bestScore = -1;

		for (uSize i = 0; i < pData->physicalDevices.Size(); i++)
		{
			const VulkanPhysicalDevice& physicalDevice = pData->physicalDevices[i];

			int32 score = 0;

			uInt32 versionMajor = VK_VERSION_MAJOR(physicalDevice.vkProperties.apiVersion);
			uInt32 versionMinor = VK_VERSION_MINOR(physicalDevice.vkProperties.apiVersion);

			if (versionMajor < 1) continue;
			if (versionMinor < 2) continue;

			if (physicalDevice.primaryQueueFamilyIndices.graphics == -1) continue;

			if (physicalDevice.vkProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				score += 1000;
			}
			else if (physicalDevice.vkProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
			{
				score += 100;
			}

			score += physicalDevice.vkProperties.limits.maxImageDimension2D;

			if (score > bestScore)
			{
				bestScore = score;
				bestDeviceIndex = i;
			}

			bestDeviceIndex++;
		}

		if (bestDeviceIndex == -1)
		{
			LogFatal("No suitable Vulkan devices found. Check that you have a Vulkan 1.2 supported GPU (or higher), and that your drivers are up to date.");
			return false;
		}

		pData->primaryPhysicalDevice = pData->physicalDevices[bestDeviceIndex];
		pData->primaryDevice = pData->devices[bestDeviceIndex];

		return true;
	}

	bool SetupVulkan()
	{
		gpVulkanData = &gpEntityWorld->CreateSingleton<VulkanData>();

		bool success =
			CreateVulkanInstance(gpVulkanData) &&
			QueryVulkanPhysicalDevices(gpVulkanData) &&
			CreateVulkanDevices(gpVulkanData) &&
			ChoosePrimaryDevices(gpVulkanData);

		PrintVulkanData(gpVulkanData);

		return success;
	}

	void Render(double delta)
	{

	}

	void CleanupVulkan()
	{
		for (VulkanDevice& device : gpVulkanData->devices)
		{
			vkDeviceWaitIdle(device.vkDevice);
			vkDestroyDevice(device.vkDevice, nullptr);
		}

		vkDestroyInstance(gpVulkanData->vkInstance, nullptr);
	}
}

extern "C"
{
	using namespace Quartz;

	bool QUARTZ_API SystemQuery(bool isEditor, Quartz::SystemQueryInfo& systemQuery)
	{
		systemQuery.name = "GraphicsModule";
		systemQuery.version = "1.0.0";

		return true;
	}

	bool QUARTZ_API SystemLoad(Log& engineLog, EntityWorld& entityWorld, Runtime& runtime)
	{
		Log::SetGlobalLog(engineLog);
		gpEntityWorld = &entityWorld;
		gpRuntime = &runtime;
		return true;
	}

	void QUARTZ_API SystemUnload()
	{
		CleanupVulkan();
	}

	void QUARTZ_API SystemPreInit()
	{
		if (SetupVulkan())
		{
			VulkanReady vulkanReady = { gpVulkanData };
			gpRuntime->Trigger<VulkanReady>(vulkanReady, true);
		}
	}

	void QUARTZ_API SystemInit()
	{
		gpRuntime->RegisterOnUpdate(Render);
		gpVulkanData->ready = true;
	}

	void QUARTZ_API SystemPostInit()
	{
		
	}
}