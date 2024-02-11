#pragma once

#include "GfxAPI.h"

namespace Quartz
{
	enum GraphicsAPI
	{
		GRAPHICS_API_NONE = 0,
		GRAPHICS_API_OPENGL,
		GRAPHICS_API_VULKAN,
		GRAPHICS_API_DX12,
	};

	struct GraphicsInstance
	{
		bool ready;
	};

	struct AvailableAPI
	{
		bool openGL;
		bool vulkan;
		bool d3d12;
	};

	struct Graphics
	{
		GraphicsAPI			activeApi;
		GraphicsInstance*	pInstance;
		AvailableAPI		available;
	};

	bool QUARTZ_GRAPHICS_API StartOpenGL();
	bool QUARTZ_GRAPHICS_API StopOpenGL();

	bool QUARTZ_GRAPHICS_API StartVulkan();
	bool QUARTZ_GRAPHICS_API StopVulkan();

	bool QUARTZ_GRAPHICS_API StartD3D12();
	bool QUARTZ_GRAPHICS_API StopD3D12();
}