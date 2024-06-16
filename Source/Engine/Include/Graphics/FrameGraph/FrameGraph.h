#pragma once

#include "EngineAPI.h"
#include "FrameGraphPass.h"
#include "Graphics/ApiInfo.h"
#include "Types/Set.h"
#include "Types/Stack.h"
#include "Window.h"
#include <functional>

namespace Quartz
{
	class QUARTZ_ENGINE_API FrameGraph
	{
	public:
		friend class Graphics;
		friend class FrameGraphPass;

	public:
		struct FrameGraphDependancy
		{
			uInt16 passIdx;
			uInt16 dependentIdx;

			friend bool operator==(const FrameGraphDependancy& dep0, const FrameGraphDependancy& dep1);
		};

	protected:
		Array<FrameGraphPass, 64>						mPasses;

		Map<String, uInt64, 128>						mResourceIds;

		Map<uInt64, FrameGraphImage, 64>				mPermanentImages;
		Map<uInt64, FrameGraphBuffer, 64>				mPermanentBuffers;
		Map<uInt64, FrameGraphImage, 128>				mTransientImages;
		Map<uInt64, FrameGraphBuffer, 128>				mTransientBuffers;

		FrameGraphCommandRecorder						mCommandRecorder;

		Array<FrameGraphResource*, 16>					mOutputResources;
		Map<uInt64, Window*, 16>						mWindowOutputs;

		Array<Array<FrameGraphDependancy, 64>, 4>		mDependencies;
		Array<Stack<FrameGraphPass*, 64>, 4>			mPassStacks;
		Array<uInt16, 4>								mOutPassIndices;
		Array<uInt16, 64>								mOrderedPasses;

		uSize											mNextResourceId;

	protected:
		virtual bool				ApiInitialize(GraphicsApiInfo& info) = 0;
		virtual bool				ApiDestroy() = 0;
		virtual ApiImageHandle		ApiAquireImage(const FrameGraphImage& graphImage) = 0;
		virtual void				ApiReleaseImage(const FrameGraphImage& graphImage) = 0;
		virtual ApiBufferHandle		ApiAquireBuffer(const FrameGraphBuffer& graphBuffer) = 0;
		virtual void				ApiReleaseBuffer(const FrameGraphBuffer& graphBuffer) = 0;
		virtual ApiRecorderHandle	ApiAquireRecorder(const FrameGraphCommandRecorder& graphRecorder) = 0;
		virtual void				ApiReleaseRecorder(const FrameGraphCommandRecorder& graphRecorder) = 0;
		virtual void				ApiExecuteFrame() = 0;

	protected:
		FrameGraphImage& AquireImage(const String& name, const FrameGraphImageInfo& imageInfo, bool& found);
		FrameGraphBuffer& AquireBuffer(const String& name, const FrameGraphBufferInfo& bufferInfo, bool& found);

		void ReleaseImage(FrameGraphImage& image);
		void ReleaseBuffer(FrameGraphBuffer& buffer);

		uSize FindDependencies(FrameGraphPass& pass, FrameGraphImage& imageInput, 
			Array<FrameGraphDependancy, 64>& outDependencies, Stack<FrameGraphPass*, 64>& outPassStack);

		void CheckActivity();

		void SortPasses();
		void AllocateResources();

		void Destroy();

		FrameGraphImage* GetImageByName(const WrapperString& name);

	public:
		FrameGraph();

		FrameGraphPass& AddPass(const String& name, const FrameGraphPassInfo& passInfo);

		bool SetOutput(const String& imageName, Window& outputWindow);
		//bool SetResourceOutputImage(const String& name);
		//bool SetResourceOutputBuffer(const String& name);

		void Reset();
		void Build();
		void Execute();
	};
}