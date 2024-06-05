#pragma once

#include "Types/Types.h"
#include "Types/String.h"
#include "Types/Array.h"
#include "Types/Set.h"

#include "Resource/Common.h"

namespace Quartz
{
	class FrameGraph;
	class FrameGraphPass;

	using PhysicalImageHandle			= void*;
	using PhysicalBufferHandle			= void*;
	using PhysicalCommandBufferHandle	= void*;

	struct FrameGraphImageInfo
	{
		uInt32			width;
		uInt32			height;
		uInt32			depth;
		uInt32			layers;
		uInt32			mipLevels;
		ImageType		type;
		ImageFormat		format;

		enum FrameGraphImageInfoFlagBits
		{
			FG_IMAGE_FLAG_PERSISTANT = 0x1 << 0
		};
		using FrameGraphImageInfoFlags = flags32;
		
		FrameGraphImageInfoFlags flags;
	};

	struct FrameGraphBufferInfo
	{
		uInt64 sizeBytes;

		enum FrameGraphBufferInfoFlagBits
		{
			FG_BUFFER_FLAG_PERSISTANT = 0x1 << 0
		};
		using FrameGraphBufferInfoFlags = flags32;

		FrameGraphBufferInfoFlags flags;
	};

	struct FrameGraphResource
	{
		String						name;
		QueueFlags					queues;
		ShaderStageFlags			stages;
		Array<FrameGraphPass*, 16>	readPasses;
		Array<FrameGraphPass*, 16>	writePasses;
	};

	struct FrameGraphImage;
	struct FrameGraphBuffer;

	struct FrameGraphImageTransition
	{
		FrameGraphImage*			pImage;
		ShaderStage					stage;
		ImageUsage					usage;
		AccessFlagBits				access;
	};

	struct FrameGraphBufferTransition
	{
		FrameGraphBuffer*			pBuffer;
		ShaderStage					stage;
		AccessFlagBits				access;
	};

	struct FrameGraphImage : public FrameGraphResource
	{
		FrameGraphImageInfo			info;
		ImageUsageFlags				usages;
		PhysicalImageHandle			hPhysicalImage;
		FrameGraphImageTransition	transitionState;
	};

	struct FrameGraphBuffer : public FrameGraphResource
	{
		FrameGraphBufferInfo		info;
		BufferUsageFlags			usages;
		PhysicalBufferHandle		hPhysicalBuffer;
	};

	struct FrameGraphCommandBuffer
	{
		QueueFlags					capableQueues;
		PhysicalCommandBufferHandle	hPhysicalCommandBuffer;
	};
}