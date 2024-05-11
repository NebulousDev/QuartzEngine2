#include "Resource/Loaders/ModelHandler.h"

#include "Log.h"

#include "ObjHelper.h"

namespace Quartz
{
	ModelHandler::ModelHandler() :
		mBufferPool(256 * MEBIBYTE),
		mModelPool(1024) { }

	bool ModelHandler::LoadAsset(File& assetFile, Asset*& pOutAsset)
	{
		String modelExt = assetFile.GetExtention();

		if (modelExt == "obj"_STR)
		{
			if (!assetFile.IsValid())
			{
				LogError("Error loading obj file [%s]. Invalid file.", assetFile.GetPath().Str());
				return false;
			}

			if (!assetFile.IsOpen())
			{
				if (!assetFile.Open(FILE_OPEN_READ))
				{
					LogError("Error loading obj file [%s]. Open() failed.", assetFile.GetPath().Str());
					return false;
				}
			}
			else
			{
				LogWarning("Obj file [%s] was already open.", assetFile.GetPath().Str());
			}

			ObjModel* pObjModel = new ObjModel();

			String objStr(assetFile.GetSize() + 1);
			if (!assetFile.Read(objStr.Data(), assetFile.GetSize()))
			{
				LogError("Error reading obj file [%s].", assetFile.GetPath().Str());
				delete pObjModel;
				return false;
			}

			if (!ParseOBJ(objStr, *pObjModel))
			{
				LogError("Error. Failed to parse obj file [%s].", assetFile.GetPath().Str());
				delete pObjModel;
				return false;
			}

			// TODO: using Indices could be too large
			ByteBuffer* pVertexBuffer	= mBufferPool.Allocate(pObjModel->indices.Size() * sizeof(float) * 6); 
			ByteBuffer* pIndexBuffer	= mBufferPool.Allocate(pObjModel->indices.Size() * sizeof(uInt32));

			if (!pVertexBuffer || !pIndexBuffer)
			{
				LogError("Error. Failed to allocate buffer space [%d MiB] for obj file [%s].", 
					(pObjModel->positions.Size() + pObjModel->indices.Size()) / 1024, assetFile.GetPath().Str());
				delete pObjModel;
				return false;
			}

			Model* pModel = mModelPool.Allocate();
			pModel->data.pVertexBuffer = pVertexBuffer;
			pModel->data.pIndexBuffer = pIndexBuffer;

			if (!ConvertOBJToModel(*pObjModel, *pModel))
			{
				LogError("Error. Failed to parse obj file [%s] into buffers.", assetFile.GetPath().Str());
				mModelPool.Free(pModel);
				mBufferPool.Free(pVertexBuffer);
				mBufferPool.Free(pIndexBuffer);
				delete pObjModel;
				return false;
			}

			pOutAsset = static_cast<Asset*>(pModel);
			delete pObjModel;

			return true;
		}

		return false;
	}

	bool ModelHandler::UnloadAsset(Asset* pInAsset)
	{
		Model* pModel = static_cast<Model*>(pInAsset);

		mBufferPool.Free(pModel->data.pVertexBuffer);
		mBufferPool.Free(pModel->data.pIndexBuffer);
		mModelPool.Free(pModel);

		return false;
	}
}