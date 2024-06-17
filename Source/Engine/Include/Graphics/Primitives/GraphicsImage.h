#pragma once

#include "Graphics/Primitives/GraphicsObject.h"
#include "Resource/Common.h"

namespace Quartz
{
	struct GraphicsImageInfo
	{
		uInt32			width;
		uInt32			height;
		uInt32			depth;
		uInt32			layers;
		uInt32			mips;
		ImageType		type;
		ImageFormat		format;
		ImageUsageFlags	usages;
	};

	class QUARTZ_ENGINE_API GraphicsImage : public GraphicsObject
	{
	private:
		GraphicsImageInfo mInfo;

	public:
		inline GraphicsImage() = default;
		inline GraphicsImage(const GraphicsImageInfo& info, const GraphicsMemoryInfo& memoryInfo, void* pNativeObject) :
			GraphicsObject(memoryInfo, pNativeObject),
			mInfo(info) {}

		inline ~GraphicsImage() { mID = 0; }

		inline bool IsValid() const override { return mID != 0; } // @TODO

		inline bool Is1D() const { return mInfo.type == IMAGE_TYPE_1D; }
		inline bool Is2D() const { return mInfo.type == IMAGE_TYPE_2D; }
		inline bool Is3D() const { return mInfo.type == IMAGE_TYPE_3D; }

		inline bool IsHDR() const { return false; } // @TODO

		inline uInt32 GetWidth() const { return mInfo.width; }
		inline uInt32 GetHeight() const { return mInfo.height; }
		inline uInt32 GetDepth() const { return mInfo.depth; }
		inline uInt32 GetLayers() const { return mInfo.layers; }
		inline uInt32 GetMips() const { return mInfo.mips; }

		inline ImageType	GetType() const { return mInfo.type; }
		inline ImageFormat	GetFormat() const { return mInfo.format; }

		inline GraphicsImageInfo GetInfo() const { return mInfo; }
	};
}