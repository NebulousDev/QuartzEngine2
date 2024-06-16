#pragma once

#include "FrameGraphResources.h"
#include "Types/Map.h"
#include "Types/String.h"
#include "Memory/PoolAllocator.h"

#include <functional>

#include "EngineAPI.h"

namespace Quartz
{
	class FrameGraph;

	struct FrameGraphPassInfo
	{
		QueueFlags queueFlags;
	};

	class QUARTZ_ENGINE_API FrameGraphPass
	{
	public:
		friend class FrameGraph;

		using PassExecuteFunc = std::function<void(
			FrameGraph& frameGraph, 
			const FrameGraphPass& framePass, 
			FrameGraphCommandRecorder& graphRecorder)>;

	private:
		FrameGraph*								mpFrameGraph;
		uInt16									mPassIdx;

		QueueFlags								mQueues;

		Array<FrameGraphImage*,  8>				mColorInputs;
		Array<FrameGraphImage*,  8>				mColorOutputs;
		Array<FrameGraphImage*,  8>				mDepthInputs;
		Array<FrameGraphImage*,  8>				mDepthOutputs;
		Array<FrameGraphImage*,  8>				mDepthStencilInputs;
		Array<FrameGraphImage*,  8>				mDepthStencilOutputs;
		Array<FrameGraphBuffer*, 8>				mVertexInputs;
		Array<FrameGraphBuffer*, 8>				mIndexInputs;
		Array<FrameGraphImage*,  8>				mTextureInputs;
		Array<FrameGraphBuffer*, 8>				mUniformInputs;

		Array<FrameGraphImageTransition, 16>	mImageTransitions;
		Array<FrameGraphBufferTransition, 16>	mBufferTransitions;

		PassExecuteFunc							mPassExecuteFunc;

	private:
		FrameGraphPass(FrameGraph* pFrameGraph, uInt16 passIdx, const FrameGraphPassInfo& passInfo);

	public:
		FrameGraphPass() = default;

		FrameGraphImage&	AddColorInput(const String& name, const FrameGraphImageInfo& imageInfo);
		FrameGraphImage&	AddColorOutput(const String& name, const FrameGraphImageInfo& imageInfo);
		FrameGraphImage&	AddDepthInput(const String& name, const FrameGraphImageInfo& imageInfo);
		FrameGraphImage&	AddDepthOutput(const String& name, const FrameGraphImageInfo& imageInfo);
		FrameGraphImage&	AddDepthStencilInput(const String& name, const FrameGraphImageInfo& imageInfo);
		FrameGraphImage&	AddDepthStencilOutput(const String& name, const FrameGraphImageInfo& imageInfo);

		FrameGraphBuffer&	AddVertexBufferInput(const String& name, const FrameGraphBufferInfo& bufferInfo);
		FrameGraphBuffer&	AddIndexBufferInput(const String& name, const FrameGraphBufferInfo& bufferInfo);

		FrameGraphImage&	AddUniformTextureInput(const String& name, const FrameGraphImageInfo& imageInfo, ShaderStageFlags shaderStages);
		FrameGraphBuffer&	AddUniformBufferInput(const String& name, const FrameGraphBufferInfo& bufferInfo, ShaderStageFlags shaderStages);

		void				SetPassExecute(PassExecuteFunc executeFunc);

		inline uInt16		GetPassIndex() const { return mPassIdx; }

		void				Execute(FrameGraphCommandRecorder& commandRecorder) const;

		inline const Array<FrameGraphImageTransition, 16>&	GetImageTransitions() const { return mImageTransitions; }
		inline const Array<FrameGraphBufferTransition, 16>& GetBufferTransitions() const { return mBufferTransitions; }
	};
}