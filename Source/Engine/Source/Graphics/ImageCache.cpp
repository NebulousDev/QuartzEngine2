#include "Graphics/ImageCache.h"

#include "Engine.h"
#include "Math/Math.h"

namespace Quartz
{
	// NOTE: key0 is compared to key1 (order dependant check)
	bool ImageInfoCompatible(const GraphicsImageCache::ImageCacheKey& key0, const GraphicsImageCache::ImageCacheKey& key1)
	{
		return
			key0.info.format == key1.info.format &&
			key0.info.width <= key1.info.width &&
			key0.info.height <= key1.info.height &&
			key0.info.depth <= key1.info.depth &&
			key0.info.layers <= key1.info.layers &&
			key0.info.mips <= key1.info.mips &&
			(uInt32)key0.info.type <= (uInt32)key1.info.type &&
			(key0.info.usages & key1.info.usages) == key0.info.usages;
	}

	bool ImageInfoEqual(const GraphicsImageCache::ImageCacheKey& key0, const GraphicsImageCache::ImageCacheKey& key1)
	{
		return
			key0.info.format == key1.info.format &&
			key0.info.width == key1.info.width &&
			key0.info.height == key1.info.height &&
			key0.info.depth == key1.info.depth &&
			key0.info.layers == key1.info.layers &&
			key0.info.mips == key1.info.mips &&
			(uInt32)key0.info.type == (uInt32)key1.info.type &&
			key0.info.usages == key1.info.usages;
	}

	bool operator==(const GraphicsImageCache::ImageCacheKey& key0, const GraphicsImageCache::ImageCacheKey& key1)
	{
		return ImageInfoEqual(key0, key1);
	}

	template<>
	hash64 Hash<GraphicsImageCache::ImageCacheKey>(const GraphicsImageCache::ImageCacheKey& key)
	{
		return
			Hash((uInt64)key.info.format) ^
			Hash((uInt64)key.info.width) ^
			Hash((uInt64)key.info.height) ^
			Hash((uInt64)key.info.depth) ^
			Hash((uInt64)key.info.layers) ^
			Hash((uInt64)key.info.mips) ^
			Hash((uInt64)key.info.type) ^
			Hash((uInt64)key.info.usages);
	}

	GraphicsImage* GraphicsImageCache::CreateImage(const GraphicsImageInfo& imageInfo)
	{
		GraphicsMemoryInfo memoryInfo;
		void* pNativeImage;

		if (!Engine::GetGraphics().ApiCreateImage(imageInfo, memoryInfo, pNativeImage))
		{
			// @TODO: Error
		}

		GraphicsImage* pImage = mImageAllocator.Allocate(imageInfo, memoryInfo, pNativeImage);

		if (!pImage)
		{
			// @TODO: out of memory
			return nullptr;
		}

		if (!pImage->IsValid())
		{
			// @TODO: Error
			return nullptr;
		}

		return pImage;
	}

	void GraphicsImageCache::DestroyImage(GraphicsImage* pImage)
	{
		Engine::GetGraphics().ApiDestroyImage(pImage->GetNativeObject());
		mImageAllocator.Free(pImage);
	}

	GraphicsImageCache::GraphicsImageCache() :
		mImageAllocator(1024 * sizeof(GraphicsImage)) { }

	void GraphicsImageCache::Update(double deltaTime)
	{
		for (uSize i = 0; i < mImages.Size(); i++)
		{
			GraphicsImage* pImage = mImages[i];

			uInt16 accessTime = 0;

			if (!(pImage->GetHints() & GRAPHICS_HINT_DONT_DESTROY))
			{
				accessTime = pImage->IncrementAccessTime();
			}

			if (accessTime > 3)
			{
				DestroyImage(pImage);

				// Fast remove swaps with the last element, so as long as
				// we decrement i, there is no issue removing in a loop.
				mImages.FastRemoveIndex(i--);
			}
		}
	}

	GraphicsImage& GraphicsImageCache::AquireImage(const GraphicsImageInfo& imageInfo)
	{
		GraphicsImage* pImage = CreateImage(imageInfo);
		mImages.PushBack(pImage);
		return *pImage;
	}

	GraphicsImage& GraphicsImageCache::AquireImage(uInt32 width, uInt32 height, ImageFormat format)
	{
		uInt32 mips = (uInt32)floor(log2f(Max(width, height)));

		GraphicsImageInfo imageInfo = {};
		imageInfo.width		= width;
		imageInfo.height	= height;
		imageInfo.depth		= 1;
		imageInfo.layers	= 1;
		imageInfo.mips		= mips;
		imageInfo.type		= IMAGE_TYPE_2D;
		imageInfo.format	= format;

		return AquireImage(imageInfo);
	}

	void GraphicsImageCache::ReleaseImage(GraphicsImage& image, bool now)
	{
		if (image.IsValid() && !(image.GetHints() & GRAPHICS_HINT_DONT_DESTROY))
		{
			if (now)
			{
				DestroyImage(&image);
			}
			else
			{
				image.SetAccessTime(9999);
			}
		}
	}
}