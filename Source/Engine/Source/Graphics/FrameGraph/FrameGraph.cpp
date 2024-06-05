#include "Graphics/FrameGraph/FrameGraph.h"

#include "Log.h"

namespace Quartz
{
	template<>
	inline hash64 Hash<FrameGraph::FrameGraphDependancy>(const FrameGraph::FrameGraphDependancy& value)
	{
		return Hash<uInt32>((uInt32)value.passIdx & (((uInt32)value.dependentIdx) << 16));
	}

	bool operator==(const FrameGraph::FrameGraphDependancy& dep0, const FrameGraph::FrameGraphDependancy& dep1)
	{
		return dep0.passIdx == dep1.passIdx && dep0.dependentIdx == dep1.dependentIdx;
	}

	FrameGraph::FrameGraph()
	{
		mCommandBuffer.capableQueues			= QUEUE_GRAPHICS | QUEUE_PRESENT;
		mCommandBuffer.hPhysicalCommandBuffer	= nullptr;
	}

	FrameGraphImage& FrameGraph::FindOrCreateImage(const String& name, const FrameGraphImageInfo& imageInfo, bool& found)
	{
		auto& imageIt = mFrameImages.Find(name);
		if (imageIt != mFrameImages.End())
		{
			found = true;
			return imageIt->value;
		}
		else
		{
			FrameGraphImage& image = mFrameImages.Put(name, FrameGraphImage());

			image.name						= name;
			image.info						= imageInfo;
			image.transitionState.pImage	= &image;
			image.transitionState.access	= ACCESS_NONE;
			image.transitionState.stage		= SHADER_STAGE_TOP_OF_PIPE;
			image.transitionState.usage		= IMAGE_USAGE_NONE;

			found = false;

			return image;
		}
	}

	FrameGraphBuffer& FrameGraph::FindOrCreateBuffer(const String& name, const FrameGraphBufferInfo& bufferInfo, bool& found)
	{
		auto& bufferIt = mFrameBuffers.Find(name);
		if (bufferIt != mFrameBuffers.End())
		{
			found = true;
			return bufferIt->value;
		}
		else
		{
			FrameGraphBuffer& buffer = mFrameBuffers.Put(name, FrameGraphBuffer());
			buffer.info = bufferInfo;
			found = false;
			return buffer;
		}
	}

	FrameGraphPass& FrameGraph::AddPass(const String& name, const FrameGraphPassInfo& passInfo)
	{
		return mPasses.PushBack(FrameGraphPass(this, mPasses.Size(), passInfo));
	}

	// Returns true if resource was found
	bool FrameGraph::SetOutputResource(const String& name)
	{
		auto& imageIt = mFrameImages.Find(name);
		if (imageIt != mFrameImages.End())
		{
			mOutputResources.PushBack(&imageIt->value);
			return true;
		}

		auto& bufferIt = mFrameBuffers.Find(name);
		if (bufferIt != mFrameBuffers.End())
		{
			mOutputResources.PushBack(&imageIt->value);
			return true;
		}

		return false;
	}

	uSize FrameGraph::FindDependencies(FrameGraphPass& pass, FrameGraphImage& imageInput, 
		Array<FrameGraphDependancy, 64>& outDependencies, Stack<FrameGraphPass*, 64>& outPassStack)
	{
		uSize dependencyCount = 0;

		FrameGraphDependancy dependency = {};
		dependency.passIdx = pass.mPassIdx;

		for (FrameGraphPass& dependantPass : mPasses)
		{
			if (pass.mPassIdx == dependantPass.mPassIdx)
			{
				continue;
			}

			for (FrameGraphImage* pColorImage : dependantPass.mColorOutputs)
			{
				if (pColorImage->name == imageInput.name)
				{
					dependency.dependentIdx = dependantPass.mPassIdx;

					outDependencies.PushBackUnique(dependency);

					if (!outPassStack.Contains(&dependantPass))
					{
						outPassStack.Push(&dependantPass);
					}

					dependencyCount++;
				}
			}

			for (FrameGraphImage* pDepthImage : dependantPass.mDepthOutputs)
			{
				if (pDepthImage->name == imageInput.name)
				{
					dependency.dependentIdx = dependantPass.mPassIdx;

					outDependencies.PushBackUnique(dependency);

					if (!outPassStack.Contains(&dependantPass))
					{
						outPassStack.Push(&dependantPass);
					}

					dependencyCount++;
				}
			}

			for (FrameGraphImage* pDepthStencilImage : dependantPass.mDepthStencilOutputs)
			{
				if (pDepthStencilImage->name == imageInput.name)
				{
					dependency.dependentIdx = dependantPass.mPassIdx;

					outDependencies.PushBackUnique(dependency);

					if (!outPassStack.Contains(&dependantPass))
					{
						outPassStack.Push(&dependantPass);
					}

					dependencyCount++;
				}
			}
		}

		return dependencyCount;
	}

	void FrameGraph::SortPasses(Array<uInt16, 64>& outPasses)
	{
		for (uSize i = 0; i < mOutputResources.Size(); i++)
		{
			const FrameGraphResource& outputResource		= *mOutputResources[i];
			Array<FrameGraphDependancy, 64>& dependencies	= mDependencies[i];
			Stack<FrameGraphPass*, 64>& passStack			= mPassStacks[i];
			uInt16& outPassIdx								= mOutPassIndices[i];

			if (outputResource.writePasses.Size() == 0)
			{
				LogFatal("Error building Frame Graph: No pass writes to output resource [%s]!", outputResource.name.Str());
				return;
			}
			else if (outputResource.writePasses.Size() > 1)
			{
				LogFatal("Error building Frame Graph: More than one pass writes to output resource [%s]!", outputResource.name.Str());
				return;
			}

			passStack.Push(outputResource.writePasses[0]);
			outPassIdx = outputResource.writePasses[0]->mPassIdx;
		
			uSize iters = 0;
			while (!passStack.IsEmpty())
			{
				FrameGraphPass& pass = *passStack.Pop();
			
				for (FrameGraphImage* pColorImageInput : pass.mColorInputs)
				{
					FindDependencies(pass, *pColorImageInput, dependencies, passStack);
				}

				for (FrameGraphImage* pDepthImageInput : pass.mDepthInputs)
				{
					FindDependencies(pass, *pDepthImageInput, dependencies, passStack);
				}

				for (FrameGraphImage* pDepthStencilImageInput : pass.mDepthStencilInputs)
				{
					FindDependencies(pass, *pDepthStencilImageInput, dependencies, passStack);
				}

				for (FrameGraphImage* pTextureImageInput : pass.mTextureInputs)
				{
					FindDependencies(pass, *pTextureImageInput, dependencies, passStack);
				}

				iters++;

				if (iters > 128)
				{
					LogError("Error building Frame Graph: Cyclical dependancies detected!");
					break;
				}
			}

			for (sSize i = dependencies.Size() - 1; i >= 0; i--)
			{
				uSize passIdx = dependencies[i].dependentIdx;

				if (!outPasses.Contains(passIdx))
				{
					outPasses.PushBack(passIdx);
				}
			}

			outPasses.PushBack(outPassIdx);
		}
	}

	void FrameGraph::AllocateResources()
	{
		for (auto& imagePair : mFrameImages)
		{
			FrameGraphImage& image	= imagePair.value;
			image.hPhysicalImage	= mFunctions.createPhysicalImageFunc(image);
		}
	}

	void FrameGraph::Build()
	{
		if (mOutputResources.Size() == 0)
		{
			LogFatal("Error building Frame Graph: No output resources set!");
			return;
		}

		mDependencies.Clear();
		mPassStacks.Clear();
		mOutPassIndices.Clear();
		mOrderedPasses.Clear();

		mDependencies.Resize(mOutputResources.Size());
		mPassStacks.Resize(mOutputResources.Size());
		mOutPassIndices.Resize(mOutputResources.Size());

		Array<uInt16, 64> orderedPasses;

		SortPasses(orderedPasses);

		AllocateResources();

		if (!mCommandBuffer.hPhysicalCommandBuffer)
		{
			mCommandBuffer.hPhysicalCommandBuffer = mFunctions.createPhysicalCommandBufferFunc(mCommandBuffer);
		}

		mFunctions.beginCommandBufferFunc(mCommandBuffer);

		for (uSize i = 0; i < orderedPasses.Size(); i++)
		{
			const FrameGraphPass& pass = mPasses[orderedPasses[i]];

			for (const FrameGraphImageTransition& imageTransition : pass.mImageTransitions)
			{
				const FrameGraphImageTransition& oldState = imageTransition.pImage->transitionState;
				const FrameGraphImageTransition& newState = imageTransition;

				mFunctions.transitionImageFunc(mCommandBuffer, oldState, newState);
			}

			pass.mPassRenderFunc(pass, mCommandBuffer);
		}

		mFunctions.endCommandBufferFunc(mCommandBuffer);
		mFunctions.submitCommandBufferFunc(mCommandBuffer);
	}

	void FrameGraph::SetFunctions(const FrameGraphFunctions& functions)
	{
		mFunctions = functions;
	}
}