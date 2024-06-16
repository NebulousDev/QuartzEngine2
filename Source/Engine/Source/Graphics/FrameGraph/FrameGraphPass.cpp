#include "Graphics/FrameGraph/FrameGraphPass.h"

#include "Graphics/FrameGraph/FrameGraph.h"

namespace Quartz
{
	// @NOTE: mPassBuffers and mPassImages should never be resized during pass generation
	FrameGraphPass::FrameGraphPass(FrameGraph* pFrameGraph, uInt16 passIdx, const FrameGraphPassInfo& passInfo) :
		mpFrameGraph(pFrameGraph),
		mPassIdx(passIdx),
		mQueues(passInfo.queueFlags) { }

	FrameGraphImage& FrameGraphPass::AddColorInput(const String& name, const FrameGraphImageInfo& imageInfo)
	{
		bool imageFound = false;
		FrameGraphImage& image = mpFrameGraph->AquireImage(name, imageInfo, imageFound);

		image.queues	|= mQueues;
		image.usages	|= IMAGE_USAGE_COLOR_INPUT;

		image.readPasses.PushBack(this);

		mColorInputs.PushBack(&image);

		FrameGraphImageTransition imageTransition = {};
		imageTransition.pImage	= &image;
		imageTransition.access	= ACCESS_COLOR_INPUT_READ;
		imageTransition.stage	= SHADER_STAGE_FRAGMENT;
		imageTransition.usage	= IMAGE_USAGE_COLOR_INPUT;

		mImageTransitions.PushBack(imageTransition);

		return image;
	}

	FrameGraphImage& FrameGraphPass::AddColorOutput(const String& name, const FrameGraphImageInfo& imageInfo)
	{
		bool imageFound = false;
		FrameGraphImage& image = mpFrameGraph->AquireImage(name, imageInfo, imageFound);

		image.queues	|= mQueues;
		image.usages	|= IMAGE_USAGE_COLOR_OUTPUT;

		image.writePasses.PushBack(this);

		mColorOutputs.PushBack(&image);

		FrameGraphImageTransition imageTransition = {};
		imageTransition.pImage	= &image;
		imageTransition.access	= ACCESS_COLOR_INPUT_WRITE;
		imageTransition.stage	= SHADER_STAGE_FRAGMENT;
		imageTransition.usage	= IMAGE_USAGE_COLOR_OUTPUT;

		mImageTransitions.PushBack(imageTransition);

		return image;
	}

	FrameGraphImage& FrameGraphPass::AddDepthInput(const String& name, const FrameGraphImageInfo& imageInfo)
	{
		bool imageFound = false;
		FrameGraphImage& image = mpFrameGraph->AquireImage(name, imageInfo, imageFound);

		image.queues	|= mQueues;
		image.usages	|= IMAGE_USAGE_DEPTH_INPUT;

		image.readPasses.PushBack(this);

		mDepthInputs.PushBack(&image);

		FrameGraphImageTransition imageTransition = {};
		imageTransition.pImage	= &image;
		imageTransition.access	= ACCESS_DEPTH_INPUT_READ;
		imageTransition.stage	= SHADER_STAGE_FRAGMENT;
		imageTransition.usage	= IMAGE_USAGE_DEPTH_INPUT;

		mImageTransitions.PushBack(imageTransition);

		return image;
	}

	FrameGraphImage& FrameGraphPass::AddDepthOutput(const String& name, const FrameGraphImageInfo& imageInfo)
	{
		bool imageFound = false;
		FrameGraphImage& image = mpFrameGraph->AquireImage(name, imageInfo, imageFound);

		image.queues	|= mQueues;
		image.usages	|= IMAGE_USAGE_DEPTH_OUTPUT;

		image.writePasses.PushBack(this);

		mDepthOutputs.PushBack(&image);

		FrameGraphImageTransition imageTransition = {};
		imageTransition.pImage	= &image;
		imageTransition.access	= ACCESS_DEPTH_INPUT_WRITE;
		imageTransition.stage	= SHADER_STAGE_FRAGMENT;
		imageTransition.usage	= IMAGE_USAGE_DEPTH_OUTPUT;

		mImageTransitions.PushBack(imageTransition);

		return image;
	}

	FrameGraphImage& FrameGraphPass::AddDepthStencilInput(const String& name, const FrameGraphImageInfo& imageInfo)
	{
		bool imageFound = false;
		FrameGraphImage& image = mpFrameGraph->AquireImage(name, imageInfo, imageFound);

		image.queues	|= mQueues;
		image.usages	|= IMAGE_USAGE_DEPTH_STENCIL_INPUT;

		image.readPasses.PushBack(this);

		mDepthStencilInputs.PushBack(&image);

		FrameGraphImageTransition imageTransition = {};
		imageTransition.pImage	= &image;
		imageTransition.access	= ACCESS_DEPTH_STENCIL_INPUT_READ;
		imageTransition.stage	= SHADER_STAGE_FRAGMENT;
		imageTransition.usage	= IMAGE_USAGE_DEPTH_STENCIL_INPUT;

		mImageTransitions.PushBack(imageTransition);

		return image;
	}

	FrameGraphImage& FrameGraphPass::AddDepthStencilOutput(const String& name, const FrameGraphImageInfo& imageInfo)
	{
		bool imageFound = false;
		FrameGraphImage& image = mpFrameGraph->AquireImage(name, imageInfo, imageFound);

		image.queues	|= mQueues;
		image.usages	|= IMAGE_USAGE_DEPTH_STENCIL_OUTPUT;

		image.writePasses.PushBack(this);

		mDepthStencilOutputs.PushBack(&image);

		FrameGraphImageTransition imageTransition = {};
		imageTransition.pImage	= &image;
		imageTransition.access	= ACCESS_DEPTH_STENCIL_INPUT_WRITE;
		imageTransition.stage	= SHADER_STAGE_FRAGMENT;
		imageTransition.usage	= IMAGE_USAGE_DEPTH_STENCIL_OUTPUT;

		mImageTransitions.PushBack(imageTransition);

		return image;
	}

	FrameGraphBuffer& FrameGraphPass::AddVertexBufferInput(const String& name, const FrameGraphBufferInfo& bufferInfo)
	{
		bool bufferFound = false;
		FrameGraphBuffer& buffer = mpFrameGraph->AquireBuffer(name, bufferInfo, bufferFound);

		buffer.queues	|= mQueues;
		buffer.usages	|= BUFFER_USAGE_VERTEX_INPUT;

		buffer.readPasses.PushBack(this);

		mVertexInputs.PushBack(&buffer);

		FrameGraphBufferTransition bufferTransition = {};
		bufferTransition.pBuffer	= &buffer;
		bufferTransition.access		= ACCESS_VERTEX_READ;
		bufferTransition.stage		= SHADER_STAGE_VERTEX;

		mBufferTransitions.PushBack(bufferTransition);

		return buffer;
	}

	FrameGraphBuffer& FrameGraphPass::AddIndexBufferInput(const String& name, const FrameGraphBufferInfo& bufferInfo)
	{
		bool bufferFound = false;
		FrameGraphBuffer& buffer = mpFrameGraph->AquireBuffer(name, bufferInfo, bufferFound);

		buffer.queues	|= mQueues;
		buffer.usages	|= BUFFER_USAGE_INDEX_INPUT;

		buffer.readPasses.PushBack(this);

		mIndexInputs.PushBack(&buffer);

		FrameGraphBufferTransition bufferTransition = {};
		bufferTransition.pBuffer	= &buffer;
		bufferTransition.access		= ACCESS_INDEX_READ;
		bufferTransition.stage		= SHADER_STAGE_VERTEX;

		mBufferTransitions.PushBack(bufferTransition);

		return buffer;
	}

	FrameGraphImage& FrameGraphPass::AddUniformTextureInput(const String& name, const FrameGraphImageInfo& imageInfo, ShaderStageFlags shaderStages)
	{
		bool imageFound = false;
		FrameGraphImage& image = mpFrameGraph->AquireImage(name, imageInfo, imageFound);

		image.queues	|= mQueues;
		image.usages	|= IMAGE_USAGE_SAMPLED_TEXUTRE;
		image.stages	|= shaderStages;

		image.readPasses.PushBack(this);

		mTextureInputs.PushBack(&image);

		FrameGraphImageTransition imageTransition = {};
		imageTransition.pImage	= &image;
		imageTransition.access	= ACCESS_SAMPLED_READ;
		imageTransition.stage	= SHADER_STAGE_FRAGMENT;
		imageTransition.usage	= IMAGE_USAGE_SAMPLED_TEXUTRE;

		mImageTransitions.PushBack(imageTransition);

		return image;
	}

	FrameGraphBuffer& FrameGraphPass::AddUniformBufferInput(const String& name, const FrameGraphBufferInfo& bufferInfo, ShaderStageFlags shaderStages)
	{
		bool bufferFound = false;
		FrameGraphBuffer& buffer = mpFrameGraph->AquireBuffer(name, bufferInfo, bufferFound);

		buffer.queues	|= mQueues;
		buffer.usages	|= BUFFER_USAGE_UNIFORM_INPUT;
		buffer.stages	|= shaderStages;

		buffer.readPasses.PushBack(this);

		mUniformInputs.PushBack(&buffer);

		FrameGraphBufferTransition bufferTransition = {};
		bufferTransition.pBuffer	= &buffer;
		bufferTransition.access		= ACCESS_UNIFORM_READ;
		bufferTransition.stage		= SHADER_STAGE_VERTEX;

		mBufferTransitions.PushBack(bufferTransition);

		return buffer;
	}

	void FrameGraphPass::SetPassExecute(PassExecuteFunc executeFunc)
	{
		mPassExecuteFunc = executeFunc;
	}

	void FrameGraphPass::Execute(FrameGraphCommandRecorder& commandRecorder) const
	{
		mPassExecuteFunc(*mpFrameGraph, *this, commandRecorder);
	}
}