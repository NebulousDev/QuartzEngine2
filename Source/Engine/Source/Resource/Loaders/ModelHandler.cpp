#include "Resource/Loaders/ModelHandler.h"

#include "Log.h"

#include "ObjHelper.h"
#include "Resource/Binary/QModelParser.h"

namespace Quartz
{
	ModelHandler::ModelHandler() :
		mBufferPool(2048 * sizeof(ByteBuffer)),
		mModelPool(1024 * sizeof(Model)) { }

	bool ModelHandler::LoadQModelAsset(File& assetFile, Asset*& pOutAsset)
	{
		if (!assetFile.IsValid())
		{
			LogError("Error loading QModel file [%s]. Invalid file.", assetFile.GetPath().Str());
			return false;
		}

		QModelParser qModelParser(assetFile, &mModelPool, &mBufferPool);

		if (!qModelParser.Read())
		{
			LogError("Error loading QModel file [%s]. qModelParser->Read() failed.", assetFile.GetPath().Str());
			return false;
		}

		pOutAsset = static_cast<Asset*>(qModelParser.GetModel());

		return true;
	}

	bool ModelHandler::LoadOBJAsset(File& assetFile, Asset*& pOutAsset)
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
			LogWarning("The obj file [%s] was already open.", assetFile.GetPath().Str());
		}

		ObjModel* pObjModel = new ObjModel();

		String objStr(assetFile.GetSize());
		if (!assetFile.Read((uInt8*)objStr.Data(), assetFile.GetSize()))
		{
			LogError("Error reading obj file [%s].", assetFile.GetPath().Str());
			assetFile.Close();
			delete pObjModel;
			return false;
		}

		assetFile.Close();

		if (!ParseOBJ(objStr, *pObjModel))
		{
			LogError("Error. Failed to parse obj file [%s].", assetFile.GetPath().Str());
			delete pObjModel;
			return false;
		}

		uInt64 indexBufferCount = 0;

		for (const ObjObject& object : pObjModel->objects)
		{
			indexBufferCount += object.indices.Size();
		}

		// TODO: using indices could be too large
		const uInt64 vertexBufferSizeBytes = indexBufferCount * sizeof(float) * 6;
		const uInt64 indexBufferSizeBytes = indexBufferCount * sizeof(uInt32);

		ByteBuffer* pVertexBuffer	= mBufferPool.Allocate(vertexBufferSizeBytes);
		ByteBuffer* pIndexBuffer	= mBufferPool.Allocate(indexBufferSizeBytes);

		if (!pVertexBuffer || !pIndexBuffer)
		{
			//LogError("Error. Failed to allocate buffer space [%d MiB] for obj file [%s].", 
			//	(vertexBufferSizeBytes + indexBufferSizeBytes) / 1024, assetFile.GetPath().Str());
			delete pObjModel;
			return false;
		}

		Model* pModel = mModelPool.Allocate(&assetFile);
		pModel->vertexData.pVertexBuffer	= pVertexBuffer;
		pModel->vertexData.pIndexBuffer		= pIndexBuffer;

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

	bool ModelHandler::LoadAsset(File& assetFile, Asset*& pOutAsset)
	{
		String fileExt = assetFile.GetExtention();

		if (fileExt == "obj"_WRAP)
		{
			return LoadOBJAsset(assetFile, pOutAsset);
		}
		else if (fileExt == "qmodel"_WRAP)
		{
			return LoadQModelAsset(assetFile, pOutAsset);
		}
		else
		{
			LogError("Failed to load model file [%s]: unknown extension '%s'.", assetFile.GetPath().Str(), fileExt.Str());
			return false;
		}

		return false;
	}

	bool ModelHandler::UnloadAsset(Asset* pInAsset)
	{
		Model* pModel = static_cast<Model*>(pInAsset);

		mBufferPool.Free(pModel->vertexData.pVertexBuffer);
		mBufferPool.Free(pModel->vertexData.pIndexBuffer);
		mModelPool.Free(pModel);

		return true;
	}
}