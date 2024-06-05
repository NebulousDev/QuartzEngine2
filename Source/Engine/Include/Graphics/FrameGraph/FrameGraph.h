#pragma once

#include "EngineAPI.h"
#include "FrameGraphPass.h"
#include "Types/Set.h"
#include "Types/Stack.h"
#include <functional>

namespace Quartz
{
	class QUARTZ_ENGINE_API FrameGraph
	{
	public:
		friend class Graphics;
		friend class FrameGraphPass;

	public:
		using CreatePhysicalImageFunc			= std::function<PhysicalImageHandle(const FrameGraphImage& graphImage)>;
		using CreatePhysicalBufferFunc			= std::function<PhysicalBufferHandle(const FrameGraphBuffer& graphBuffer)>;
		using CreatePhysicalCommandBufferFunc	= std::function<PhysicalCommandBufferHandle(const FrameGraphCommandBuffer& graphCommandBuffer)>;

		using BeginCommandBufferFunc			= std::function<void(const FrameGraphCommandBuffer& graphCommandBuffer)>;
		using EndCommandBufferFunc				= std::function<void(const FrameGraphCommandBuffer& graphCommandBuffer)>;
		using SubmitCommandBufferFunc			= std::function<void(const FrameGraphCommandBuffer& graphCommandBuffer)>;

		using TransitionImageFunc				= std::function<void(
														FrameGraphCommandBuffer& graphCommandBuffer,
														const FrameGraphImageTransition& oldState, 
														const FrameGraphImageTransition& newState)>;

		using TransitionBufferFunc				= std::function<void(
														FrameGraphCommandBuffer& graphCommandBuffer,
														const FrameGraphBufferTransition& oldState, 
														const FrameGraphBufferTransition& newState)>;

	public:
		struct FrameGraphFunctions
		{
			CreatePhysicalImageFunc			createPhysicalImageFunc;
			CreatePhysicalBufferFunc		createPhysicalBufferFunc;
			CreatePhysicalCommandBufferFunc	createPhysicalCommandBufferFunc;
			BeginCommandBufferFunc			beginCommandBufferFunc;
			EndCommandBufferFunc			endCommandBufferFunc;
			SubmitCommandBufferFunc			submitCommandBufferFunc;
			TransitionImageFunc				transitionImageFunc;
			TransitionBufferFunc			transitionBufferFunc;
		};

		struct FrameGraphDependancy
		{
			uInt16 passIdx;
			uInt16 dependentIdx;

			friend bool operator==(const FrameGraphDependancy& dep0, const FrameGraphDependancy& dep1);
		};

	private:
		Map<String, FrameGraphImage, 128>				mFrameImages;
		Map<String, FrameGraphBuffer, 128>				mFrameBuffers;

		FrameGraphCommandBuffer							mCommandBuffer;

		Array<FrameGraphPass, 64>						mPasses; 

		FrameGraphFunctions								mFunctions;

		Array<FrameGraphResource*, 4>					mOutputResources;
		Array<Array<FrameGraphDependancy, 64>, 4>		mDependencies;
		Array<Stack<FrameGraphPass*, 64>, 4>			mPassStacks;
		Array<uInt16, 4>								mOutPassIndices;
		Array<uInt16, 64>								mOrderedPasses;

	private:
		FrameGraphImage& FindOrCreateImage(const String& name, const FrameGraphImageInfo& imageInfo, bool& found);
		FrameGraphBuffer& FindOrCreateBuffer(const String& name, const FrameGraphBufferInfo& bufferInfo, bool& found);

		uSize FindDependencies(FrameGraphPass& pass, FrameGraphImage& imageInput, 
			Array<FrameGraphDependancy, 64>& outDependencies, Stack<FrameGraphPass*, 64>& outPassStack);

		void SortPasses(Array<uInt16, 64>& outPasses);

		void AllocateResources();

	protected:
		void SetFunctions(const FrameGraphFunctions& functions);

	public:
		FrameGraph();

		FrameGraphPass& AddPass(const String& name, const FrameGraphPassInfo& passInfo);

		bool SetOutputResource(const String& name);

		void Build();

		template<typename PhysicalType>
		PhysicalType* GetPhysicalImage(const FrameGraphImage& graphImage) const { return (PhysicalType*) nullptr; };

		template<typename PhysicalType>
		PhysicalType* GetPhysicalBuffer(const FrameGraphBuffer& graphBuffer) const { return (PhysicalType*) nullptr; };

		template<typename PhysicalType>
		PhysicalType* GetPhysicalCommandBuffer(const FrameGraphCommandBuffer& graphCommandBuffer) const { return (PhysicalType*) nullptr; };
	};
}