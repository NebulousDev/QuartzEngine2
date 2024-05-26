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

		Model* pModel = mModelPool.Allocate(&assetFile);

		ObjConvertSettings objConvertSettings = {};
		objConvertSettings.writePositions		= true;
		objConvertSettings.writeNormals			= true;
		objConvertSettings.writeTangents		= false;
		objConvertSettings.writeBiTangents		= false;
		objConvertSettings.writeTexCoords		= true;
		objConvertSettings.positionStreamIdx	= 0;
		objConvertSettings.normalStreamIdx		= 0;
		objConvertSettings.tangentStreamIdx		= 0;
		objConvertSettings.biTangentStreamIdx	= 0;
		objConvertSettings.texCoordStreamIdx	= 0;
		objConvertSettings.positionFormat		= VERTEX_FORMAT_FLOAT3;
		objConvertSettings.normalFormat			= VERTEX_FORMAT_FLOAT3;
		objConvertSettings.tangentFormat		= VERTEX_FORMAT_FLOAT3;
		objConvertSettings.biTangentFormat		= VERTEX_FORMAT_FLOAT3;
		objConvertSettings.texCoordFormat		= VERTEX_FORMAT_FLOAT2;
		objConvertSettings.useTangentBiTanW		= true;
		objConvertSettings.flipNormals			= false;

		if (!ConvertOBJToModel(objConvertSettings, *pObjModel, *pModel, mBufferPool))
		{
			LogError("Error. Failed to parse obj file [%s] into buffers.", assetFile.GetPath().Str());
			
			for (VertexStream& stream : pModel->vertexStreams)
			{
				mBufferPool.Free(stream.pVertexBuffer);
			}

			mBufferPool.Free(pModel->indexStream.pIndexBuffer);

			mModelPool.Free(pModel);

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

		//mBufferPool.Free(pModel->vertexData.pVertexBuffer);
		//mBufferPool.Free(pModel->vertexData.pIndexBuffer);
		mModelPool.Free(pModel);

		return true;
	}
}