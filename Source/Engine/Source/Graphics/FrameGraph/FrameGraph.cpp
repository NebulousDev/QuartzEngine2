#include "Graphics/FrameGraph/FrameGraph.h"

#include "Log.h"
#include "Debug.h"

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

	FrameGraph::FrameGraph() :
	mNextResourceId(1) { }

	FrameGraphImage& FrameGraph::AquireImage(const String& name, const FrameGraphImageInfo& imageInfo, bool& found)
	{
		uInt64 imageId = 0;

		auto& iamgeIdIt = mResourceIds.Find(name);
		if (iamgeIdIt != mResourceIds.End())
		{
			imageId = iamgeIdIt->value;
			//goto makeImage;
		}

		auto& transImageIt = mTransientImages.Find(imageId);
		if (transImageIt != mTransientImages.End())
		{
			found = true;
			return transImageIt->value;
		}
		else
		{
			auto& permImageIt = mPermanentImages.Find(imageId);
			if (permImageIt != mPermanentImages.End())
			{
				found = true;
				return permImageIt->value;
			}
			else
			{
			makeImage:

				FrameGraphImage* pImage = nullptr;

				uInt64 newResourceId = mNextResourceId++;

				if (imageInfo.flags & RESOURCE_FLAG_PERSISTANT)
				{
					pImage = &mPermanentImages.Put(newResourceId);
				}
				else
				{
					pImage = &mTransientImages.Put(newResourceId);
				}

				pImage->id						= newResourceId;
				pImage->name					= name;
				pImage->info					= imageInfo;
				pImage->transitionState.pImage	= pImage;
				pImage->transitionState.access	= ACCESS_NONE;
				pImage->transitionState.stage	= SHADER_STAGE_TOP_OF_PIPE;
				pImage->transitionState.usage	= IMAGE_USAGE_NONE;
				pImage->flags					= imageInfo.flags;

				mResourceIds.Put(name, newResourceId);

				found = false;

				return *pImage;
			}
		}
	}

	FrameGraphBuffer& FrameGraph::AquireBuffer(const String& name, const FrameGraphBufferInfo& bufferInfo, bool& found)
	{
		uInt64 bufferId = 0;

		auto& bufferIdIt = mResourceIds.Find(name);
		if (bufferIdIt != mResourceIds.End())
		{
			bufferId = bufferIdIt->value;
			//goto makeBuffer;
		}

		auto& transBufferIt = mTransientBuffers.Find(bufferId);
		if (transBufferIt != mTransientBuffers.End())
		{
			found = true;
			return transBufferIt->value;
		}
		else
		{
			auto& permBufferIt = mPermanentBuffers.Find(bufferId);
			if (permBufferIt != mPermanentBuffers.End())
			{
				found = true;
				return permBufferIt->value;
			}
			else
			{
			makeBuffer:

				FrameGraphBuffer* pBuffer = nullptr;

				uInt64 newResourceId = mNextResourceId++;

				if (bufferInfo.flags & RESOURCE_FLAG_PERSISTANT)
				{
					pBuffer = &mPermanentBuffers.Put(newResourceId);
				}
				else
				{
					pBuffer = &mTransientBuffers.Put(newResourceId);
				}

				pBuffer->id		= newResourceId;
				pBuffer->name	= name;
				pBuffer->info	= bufferInfo;

				mResourceIds.Put(name, newResourceId);

				found = false;

				return *pBuffer;
			}
		}
	}

	FrameGraphPass& FrameGraph::AddPass(const String& name, const FrameGraphPassInfo& passInfo)
	{
		return mPasses.PushBack(FrameGraphPass(this, mPasses.Size(), passInfo));
	}

	// Returns true if resource was found
	bool FrameGraph::SetOutput(const String& imageName, Window& outputWindow)
	{
		uInt64 imageId = 0;

		auto& iamgeIdIt = mResourceIds.Find(imageName);
		if (iamgeIdIt != mResourceIds.End())
		{
			imageId = iamgeIdIt->value;

			DEBUG_ONLY
			{
				auto& imageIt = mTransientImages.Find(imageId);
				if (imageIt != mTransientImages.End())
				{
					LogError("Error setting FrameGraph output resource: Resource [type=Image, name=\"%s\"] must be marked RESOURCE_FLAG_PERSISTANT.", 
						imageName.Str());
					return false;
				}

				//auto& bufferIt = mTransientBuffers.Find(imageName);
				//if (bufferIt != mTransientBuffers.End())
				//{
				//	LogError("Error setting FrameGraph output resource: Resource [type=Buffer, name=\"%s\"] must be marked RESOURCE_FLAG_PERSISTANT.");
				//	return false;
				//}
			}

			auto& imageIt = mPermanentImages.Find(imageId);
			if (imageIt != mPermanentImages.End())
			{
				if (!mWindowOutputs.Contains(imageIt->key))
				{
					mWindowOutputs.Put(imageIt->key, &outputWindow);
					mOutputResources.PushBack(&imageIt->value);
					imageIt->value.flags |= RESOURCE_FLAG_PERSISTANT | RESOURCE_FLAG_BACKBUFFER | RESOURCE_FLAG_WINDOW_OUTPUT;
				}

				return true;
			}

			//auto& bufferIt = mPermanentBuffers.Find(imageName);
			//if (bufferIt != mPermanentBuffers.End())
			//{
			//	mOutputResources.PushBack(&imageIt->value);
			//	return true;
			//}
		}

		LogError("Error setting FrameGraph output resource: Resource [type=Image, name=\"%s\"] is not registered in any pass.", 
			imageName.Str());

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
						pColorImage->activity = 0;
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
						pDepthImage->activity = 0;
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
						pDepthStencilImage->activity = 0;
					}

					dependencyCount++;
				}
			}
		}

		return dependencyCount;
	}

	void FrameGraph::CheckActivity()
	{
		for (auto& imagePair : mTransientImages)
		{
			if (imagePair.value.activity > 2)
			{
				ApiReleaseImage(imagePair.value);
				mTransientImages.Remove(imagePair.key);
			}
			else
			{
				imagePair.value.activity++;
			}
		}

		//for (auto& bufferPair : mTransientBuffers)
		//{
		//	ApiReleaseBuffer(bufferPair.value);
		//}

		//mTransientImages.Clear();
		//mTransientBuffers.Clear();
	}

	void FrameGraph::SortPasses()
	{
		for (uSize i = 0; i < mOutputResources.Size(); i++)
		{
			FrameGraphResource& outputResource				= *mOutputResources[i];
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

				if (!mOrderedPasses.Contains(passIdx))
				{
					mOrderedPasses.PushBack(passIdx);
				}
			}

			outputResource.activity = 0;

			mOrderedPasses.PushBack(outPassIdx);
		}
	}

	void FrameGraph::AllocateResources()
	{
		for (auto& imagePair : mPermanentImages)
		{
			FrameGraphImage& image = imagePair.value;

			if (image.hApiImage == nullptr)
			{
				image.hApiImage = ApiAquireImage(image);
			}
		}

		for (auto& bufferPair : mPermanentBuffers)
		{
			FrameGraphBuffer& buffer = bufferPair.value;

			if (buffer.hApiBuffer == nullptr)
			{
				buffer.hApiBuffer = ApiAquireBuffer(buffer);
			}
		}

		for (auto& imagePair : mTransientImages)
		{
			FrameGraphImage& image = imagePair.value;
			image.hApiImage	= ApiAquireImage(image);
		}

		for (auto& bufferPair : mTransientBuffers)
		{
			FrameGraphBuffer& buffer = bufferPair.value;
			buffer.hApiBuffer = ApiAquireBuffer(buffer);
		}
	}

	void FrameGraph::Destroy()
	{
		
	}

	FrameGraphImage* FrameGraph::GetImageByName(const WrapperString& name)
	{
		uInt64 imageId = 0;

		auto& iamgeIdIt = mResourceIds.Find(name);
		if (iamgeIdIt != mResourceIds.End())
		{
			imageId = iamgeIdIt->value;
		}

		auto& transImageIt = mTransientImages.Find(imageId);
		if (transImageIt != mTransientImages.End())
		{
			return &transImageIt->value;
		}

		auto& permImageIt = mPermanentImages.Find(imageId);
		if (permImageIt != mPermanentImages.End())
		{
			return &permImageIt->value;
		}

		return nullptr;
	}

	void FrameGraph::Reset()
	{
		for (auto& imagePair : mTransientImages)
		{
			mResourceIds.Remove(imagePair.value.name);
		}

		for (auto& imagePair : mPermanentImages)
		{
			FrameGraphImage& image = imagePair.value;
			image.readPasses.Clear();
			image.writePasses.Clear();
			//image.stages = 0;
			image.usages = 0;
			image.queues = 0;
		}

		for (auto& bufferPair : mPermanentBuffers)
		{
			FrameGraphBuffer& buffer = bufferPair.value;
			buffer.readPasses.Clear();
			buffer.writePasses.Clear();
			//buffer.stages = 0;
			buffer.usages = 0;
			buffer.queues = 0;
		}

		mOutputResources.Clear();
		mWindowOutputs.Clear();

		mDependencies.Clear();
		mPassStacks.Clear();
		mOutPassIndices.Clear();
		mOrderedPasses.Clear();

		mPasses.Clear();
	}

	void FrameGraph::Build()
	{
		if (mOutputResources.Size() == 0)
		{
			LogFatal("Error building Frame Graph: No output resources set!");
			return;
		}

		mDependencies.Resize(mOutputResources.Size());
		mPassStacks.Resize(mOutputResources.Size());
		mOutPassIndices.Resize(mOutputResources.Size());

		SortPasses();
		AllocateResources();
	}

	void FrameGraph::Execute()
	{
		ApiExecuteFrame();
		CheckActivity();
	}
}