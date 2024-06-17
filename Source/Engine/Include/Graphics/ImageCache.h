#pragma once

#include "EngineAPI.h"
#include "Graphics/Primitives/GraphicsImage.h"
#include "Memory/PoolAllocator.h"
#include "Types/Map.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API GraphicsImageCache
	{
	public:
		struct ImageCacheKey
		{
			GraphicsImageInfo info;

			friend bool operator==(const ImageCacheKey& key0, const ImageCacheKey& key1);
			friend hash64 Hash(const ImageCacheKey& key);
		};

	private:
		PoolAllocator<GraphicsImage>	mImageAllocator;
		Array<GraphicsImage*>			mImages;
		Array<GraphicsImage*>			mInactiveImages;
		Map<uInt64, uInt32>				mIdImageMap;

		GraphicsImage* CreateImage(const GraphicsImageInfo& imageInfo);
		void DestroyImage(GraphicsImage* pImage);

	public:
		GraphicsImageCache();

		void Update(double deltaTime);

		GraphicsImage& AquireImage(const GraphicsImageInfo& imageInfo);
		GraphicsImage& AquireImage(uInt32 width, uInt32 height, ImageFormat format);

		void ReleaseImage(GraphicsImage& image, bool now = false);
	};
}