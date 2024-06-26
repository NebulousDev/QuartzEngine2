#include "Resource/Loaders/ImageHandler.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#include "Log.h"

namespace Quartz
{
	ImageHandler::ImageHandler() :
		mBufferPool(2048 * sizeof(ByteBuffer)),
		mImagePool(1024 * sizeof(Image))
	{
		stbi_set_flip_vertically_on_load(true);
	}

	bool ImageHandler::LoadAsset(File& assetFile, Asset*& pOutAsset)
	{
		String imageExt = assetFile.GetExtention();

		if (!assetFile.IsValid())
		{
			LogError("Error loading %s file [%s]. Invalid file.", imageExt.Str(), assetFile.GetPath().Str());
			return false;
		}

		if (assetFile.IsOpen())
		{
			LogWarning("The %s file [%s] was already open.", imageExt.Str(), assetFile.GetPath().Str());
		}

		sSize imageWidth = 0;
		sSize imageHeight = 0;
		sSize imageComponents = 0;

		stbi_uc* pImageData = stbi_load(assetFile.GetPath().Str(),
			&imageWidth, &imageHeight, &imageComponents, 4);//STBI_default);

		if (!pImageData)
		{
			const char* stbErrorMessage = stbi_failure_reason();
			LogError("Error loading %s file [%s]. stbi_load() failed with reason: %s", 
				imageExt.Str(), assetFile.GetPath().Str(), stbErrorMessage);

			return false;
		}

		ImageFormat imageFormat = IMAGE_FORMAT_INVALID;

		switch (imageComponents)
		{
			case 1: imageFormat = IMAGE_FORMAT_R8;			break;
			case 2: imageFormat = IMAGE_FORMAT_R8G8;		break;
			case 3: imageFormat = IMAGE_FORMAT_R8G8B8;		break;
			case 4: imageFormat = IMAGE_FORMAT_R8G8B8A8;	break;

		default:
			{
				LogError("Error loading %s file [%s]. Invalid component count [%d]",
					imageExt.Str(), assetFile.GetPath().Str(), imageComponents);
				break;
			}
		}

		Image* pImage = mImagePool.Allocate();

		pImage->width	= imageWidth;
		pImage->height	= imageHeight;
		pImage->depth	= 1;
		pImage->format	= imageFormat;

		uSize imageSizeBytes = imageWidth * imageHeight * 4;// pImage->FormatSize();
		ByteBuffer* pImageDataBuffer = mBufferPool.Allocate(imageSizeBytes);

		pImageDataBuffer->WriteData((void*)pImageData, imageSizeBytes);

		pImage->pImageData = pImageDataBuffer;

		stbi_image_free(pImageData);

		pOutAsset = static_cast<Asset*>(pImage);

		return true;
	}

	bool ImageHandler::UnloadAsset(Asset* pInAsset)
	{
		Image* pImage = static_cast<Image*>(pInAsset);
		
		mBufferPool.Free(pImage->pImageData);
		mImagePool.Free(pImage);

		return true;
	}
}