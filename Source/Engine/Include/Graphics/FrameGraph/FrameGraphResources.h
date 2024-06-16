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

	using ApiImageHandle	= void*;
	using ApiBufferHandle	= void*;
	using ApiRecorderHandle = void*;

	enum FrameGraphResourceFlagBits
	{
		RESOURCE_FLAG_NONE			= 0x0 << 0,
		RESOURCE_FLAG_PERSISTANT	= 0x1 << 1,
		RESOURCE_FLAG_BACKBUFFER	= 0x1 << 2,
		RESOURCE_FLAG_WINDOW_OUTPUT	= 0x1 << 3,
		RESOURCE_FLAG_IMAGE_OUTPUT	= 0x1 << 4,
		RESOURCE_FLAG_BUFFER_OUTPUT = 0x1 << 5,
	};

	using FrameGraphResourceFlags = flags32;

	struct FrameGraphImageInfo
	{
		uInt32						width;
		uInt32						height;
		uInt32						depth;
		uInt32						layers;
		uInt32						mipLevels;
		ImageType					type;
		ImageFormat					format;
		FrameGraphResourceFlags		flags;
	};

	struct FrameGraphBufferInfo
	{
		uInt64						sizeBytes;
		FrameGraphResourceFlags		flags;
	};

	struct FrameGraphResource
	{
		uInt64						id;
		String						name;
		QueueFlags					queues;
		ShaderStageFlags			stages;
		Array<FrameGraphPass*, 16>	readPasses;
		Array<FrameGraphPass*, 16>	writePasses;
		FrameGraphResourceFlags		flags;
		uInt32						activity;
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
		ApiImageHandle				hApiImage;
		FrameGraphImageTransition	transitionState;
	};

	struct FrameGraphBuffer : public FrameGraphResource
	{
		FrameGraphBufferInfo		info;
		BufferUsageFlags			usages;
		ApiBufferHandle				hApiBuffer;
	};

	struct FrameGraphCommandRecorder
	{
		uInt64						id;
		ApiRecorderHandle			hApiRecorder;
	};
}