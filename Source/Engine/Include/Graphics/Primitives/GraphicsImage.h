#include "Graphics/Primitives/GraphicsObject.h"
#include "Resource/Common.h"

namespace Quartz
{
	class GraphicsImageInfo
	{
		uInt32		width;
		uInt32		height;
		uInt32		depth;
		uInt32		layers;
		uInt32		mips;
		ImageType	type;
		ImageFormat	format;
	};

	class GraphicsImage : public GraphicsObject
	{
	private:
		uInt32		mWidth;
		uInt32		mHeight;
		uInt32		mDepth;
		uInt32		mLayers;
		uInt32		mMips;
		ImageType	mType;
		ImageFormat	mFormat;

	public:
		inline GraphicsImage() = default;
		inline GraphicsImage(const GraphicsImageInfo& info, const GraphicsMemoryInfo& memoryInfo, void* pNativeObject) :
			GraphicsObject(info, memoryInfo, pNativeObject),
			mWidth(info.width), mHeight(info.height), mDepth(info.depth)
			mLayers(info.layers), mMips(info.mips),
			mType(info.type), mFormat(info.format) {}

		inline bool IsValid() const override { return true; } // @TODO

		inline bool Is1D() const { return mType == IMAGE_TYPE_1D; }
		inline bool Is2D() const { return mType == IMAGE_TYPE_2D; }
		inline bool Is3D() const { return mType == IMAGE_TYPE_3D; }

		inline bool IsHDR() const { return false; } // @TODO

		inline uInt32 GetWidth() const { return mWidth; }
		inline uInt32 GetHeight() const { return mHeight; }
		inline uInt32 GetDepth() const { return mDepth; }
		inline uInt32 GetLayers() const { return mLayers; }
		inline uInt32 GetMips() const { return mMips; }

		inline ImageType	GetType() const { return mType; }
		inline ImageFormat	GetFormat() const { return mFormat; }
	};
}