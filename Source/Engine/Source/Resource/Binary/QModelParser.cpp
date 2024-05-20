#include "Resource/Binary/QModelParser.h"

#include "Log.h"

namespace Quartz
{
	QModel QModelParser::WriteBlankQModelHeader()
	{
		QStringTable stringTable;
		stringTable.encoding		= Q_STRING_UTF8;
		stringTable._reserved0		= 0;
		stringTable.stringCount		= 0;
		stringTable.strsOffset		= 0;
		stringTable.strsSizeBytes	= 0;
		stringTable.nextExtOffset	= 0;

		QModelMeshTable meshTable;
		meshTable.meshCount				= 0;
		meshTable._reserved0			= 0;
		meshTable.meshesOffset			= 0;
		meshTable.meshesSizeBytes		= 0;
		meshTable.vertexBufferSizeBytes	= 0;
		meshTable.indexBufferSizeBytes	= 0;
		meshTable.nextExtOffset			= 0;

		QModel qModel;				// Magic assigned in header
		qModel.versionMajor			= 1;
		qModel.versionMinor			= 3;
		qModel._reserved0			= 0;
		qModel.stringTable			= stringTable;
		qModel.meshTable			= meshTable;
		qModel.nextExtOffset		= 0;

		mFile.WriteValues<QModel>(&qModel, 1);

		return qModel;
	}

	QStringID QModelParser::RegisterString(const String& string)
	{
		mStrings.PushBack(string);
		return mStrings.Size() - 1;
	}

	bool QModelParser::BeginWriting()
	{
		if (mFile.IsOpen())
		{
			LogWarning("QModel File [%s] is already open.", mFile.GetPath().Str());
			mFile.SetFilePtr(0, FILE_PTR_BEGIN);
		}
		else if (!mFile.Open(FILE_OPEN_WRITE | FILE_OPEN_BINARY | FILE_OPEN_CLEAR | FILE_OPEN_SHARE_READ))
		{
			LogError("QModel File [%s] failed to open.", mFile.GetPath().Str());
			return false;
		}

		mHeader = WriteBlankQModelHeader();

		return true;
	}

	bool QModelParser::WriteStrings()
	{
		mHeader.stringTable.strsOffset = mFile.GetFilePtr();

		for (const String& string : mStrings)
		{
			uInt16 strLength = string.Length() + 1;

			if (!mFile.WriteValues<uInt16>(&strLength, 1))
			{
				return false;
			}

			if (!mFile.Write((uInt8*)string.Str(), string.Length() + 1))
			{
				return false;
			}

			mHeader.stringTable.stringCount++;
			mHeader.stringTable.strsSizeBytes += 2 + strLength;
		}

		return true;
	}

	bool QModelParser::EndWriting()
	{
		mFile.SetFilePtr(0, FILE_PTR_BEGIN);

		if (!mFile.WriteValues<QModel>(&mHeader, 1))
		{
			return false;
		}

		mFile.Close();

		return true;
	}

	bool QModelParser::WriteMesh(const Mesh& mesh, const VertexData& vertexData)
	{
		QModelMeshTable& meshTable = mHeader.meshTable;

		uInt64 fileOffset = mFile.GetFilePtr();

		if (meshTable.meshesOffset == 0)
		{
			meshTable.meshesOffset = fileOffset;
		}

		meshTable.vertexBufferSizeBytes += mesh.verticesSizeBytes;
		meshTable.indexBufferSizeBytes	+= mesh.indicesSizeBytes;

		QModelMesh qModelMesh;
		qModelMesh.nameID				= RegisterString(mesh.name);
		qModelMesh.materialPathID		= RegisterString(mesh.materialPath);
		qModelMesh.lodIdx				= mesh.lod;
		qModelMesh._reserved0			= 0;
		qModelMesh.indexFormat			= (QModelIndexFormat)mesh.indexElement.FormatID();
		qModelMesh.elementCount		= mesh.elements.Size();
		qModelMesh.elementOffset		= fileOffset + sizeof(QModelMesh);
		qModelMesh.verticesOffset		= qModelMesh.elementOffset + qModelMesh.elementCount * sizeof(QModelMeshElement);
		qModelMesh.verticesSizeBytes	= mesh.verticesSizeBytes;
		qModelMesh.indicesOffset		= qModelMesh.verticesOffset + qModelMesh.verticesSizeBytes;
		qModelMesh.indicesSizeBytes	= mesh.indicesSizeBytes;

		if (!mFile.WriteValues<QModelMesh>(&qModelMesh, 1))
		{
			return false;
		}

		meshTable.meshesSizeBytes += sizeof(QModelMesh);

		for (const VertexElement& vertElement : mesh.elements)
		{
			QModelMeshElement meshElement;
			meshElement.attribute	= (QModelVertexAttribute)vertElement.attribute;
			meshElement.format		= (QModelVertexFormat)vertElement.FormatID();

			if (!mFile.WriteValues<QModelMeshElement>(&meshElement, 1))
			{
				return false;
			}

			meshTable.meshesSizeBytes += sizeof(QModelMeshElement);
		}

		if (!mFile.Write(vertexData.pVertexBuffer->Data() + mesh.verticesStartBytes, mesh.verticesSizeBytes))
		{
			return false;
		}

		if (!mFile.Write(vertexData.pIndexBuffer->Data() + mesh.indicesStartBytes, mesh.indicesSizeBytes))
		{
			return false;
		}

		meshTable.meshesSizeBytes += qModelMesh.verticesSizeBytes + qModelMesh.indicesSizeBytes;
		meshTable.meshCount++;

		return true;
	}

	bool QModelParser::BeginReading()
	{
		if (mFile.IsOpen())
		{
			LogWarning("QModel File [%s] is already open.", mFile.GetPath().Str());
			mFile.SetFilePtr(0, FILE_PTR_BEGIN);
		}
		else if (!mFile.Open(FILE_OPEN_READ | FILE_OPEN_BINARY | FILE_OPEN_SHARE_READ))
		{
			LogError("Error reading QModel File [%s]. Failed to open.", mFile.GetPath().Str());
			return false;
		}

		if (!mFile.ReadValues<QModel>(&mHeader, 1))
		{
			return false;
		}

		if (WrapperString(mHeader.magic, 6) != "QModel"_STR)
		{
			LogError("Error reading QModel File [%s]. File is not a valid QModel file.", mFile.GetPath().Str());
			return false;
		}

		return true;
	}

	bool QModelParser::ReadStrings()
	{
		if (mHeader.stringTable.stringCount == 0)
		{
			return true;
		}

		mFile.SetFilePtr(mHeader.stringTable.strsOffset, FILE_PTR_BEGIN);

		for (uSize i = 0; i < mHeader.stringTable.stringCount; i++)
		{
			uInt16 strLength = 0;

			if (!mFile.ReadValues<uInt16>(&strLength, 1))
			{
				return false;
			}

			String string(strLength);

			if (!mFile.Read((uInt8*)string.Data(), strLength))
			{
				return false;
			}

			mStrings.PushBack(string);
		}

		return true;
	}

	bool QModelParser::ReadMeshes()
	{
		if (mHeader.meshTable.meshCount == 0)
		{
			return true;
		}

		mFile.SetFilePtr(mHeader.meshTable.meshesOffset, FILE_PTR_BEGIN);

		Model* pModel = mpModelAllocator->Allocate(&mFile);
		
		if (!pModel)
		{
			// @TODO: error
			return false;
		}

		VertexData& vertexData = pModel->vertexData;

		pModel->meshes.Resize(mHeader.meshTable.meshCount);

		const uInt64 vertexBufferSizeBytes	= mHeader.meshTable.vertexBufferSizeBytes;
		const uInt64 indexBufferSizeBytes	= mHeader.meshTable.indexBufferSizeBytes;

		vertexData.pVertexBuffer = mpBufferAllocator->Allocate(vertexBufferSizeBytes);
		vertexData.pVertexBuffer->Allocate(vertexBufferSizeBytes);

		vertexData.pIndexBuffer = mpBufferAllocator->Allocate(indexBufferSizeBytes);
		vertexData.pIndexBuffer->Allocate(indexBufferSizeBytes);

		uInt64 vertexBufferOffset	= 0;
		uInt64 indexBufferOffset	= 0;

		for (uSize i = 0; i < mHeader.meshTable.meshCount; i++)
		{
			Mesh& mesh = pModel->meshes[i];
			QModelMesh qModelMesh;

			if (!mFile.ReadValues<QModelMesh>(&qModelMesh, 1))
			{
				mpBufferAllocator->Free(vertexData.pVertexBuffer);
				mpBufferAllocator->Free(vertexData.pIndexBuffer);
				mpModelAllocator->Free(pModel);
				return false;
			}

			if (qModelMesh.nameID < mStrings.Size())
			{
				mesh.name = mStrings[qModelMesh.nameID];
			}
			else
			{
				mpBufferAllocator->Free(vertexData.pVertexBuffer);
				mpBufferAllocator->Free(vertexData.pIndexBuffer);
				mpModelAllocator->Free(pModel);
				return false;
			}

			if (qModelMesh.materialPathID < mStrings.Size())
			{
				mesh.materialPath = mStrings[qModelMesh.materialPathID];
			}
			else
			{
				mpBufferAllocator->Free(vertexData.pVertexBuffer);
				mpBufferAllocator->Free(vertexData.pIndexBuffer);
				mpModelAllocator->Free(pModel);
				return false;
			}

			mesh.lod				= qModelMesh.lodIdx;
			mesh.verticesStartBytes	= vertexBufferOffset;
			mesh.verticesSizeBytes	= qModelMesh.verticesSizeBytes;
			mesh.indicesStartBytes	= indexBufferOffset;
			mesh.indicesSizeBytes	= qModelMesh.indicesSizeBytes;

			mFile.SetFilePtr(qModelMesh.elementOffset, FILE_PTR_BEGIN);

			Array<QModelMeshElement, 8> meshElements(qModelMesh.elementCount);

			// @TODO: check mesh.elementCount < 8

			if (!mFile.ReadValues<QModelMeshElement>(meshElements.Data(), qModelMesh.elementCount))
			{
				mpModelAllocator->Free(pModel);
				return false;
			}
				
			for (uSize j = 0; j < meshElements.Size(); j++)
			{
				QModelMeshElement& meshElement = meshElements[j];

				VertexAttribute attribute	= VERTEX_ATTRIBUTE_INVALID;
				VertexFormat	format		= VERTEX_FORMAT_INVALID;

				attribute = (VertexAttribute)meshElement.attribute;

				switch (meshElement.format)
				{
					case QMODEL_VERTEX_FORMAT_FLOAT:			format = VERTEX_FORMAT_FLOAT;			break;
					case QMODEL_VERTEX_FORMAT_FLOAT2:			format = VERTEX_FORMAT_FLOAT2;			break;
					case QMODEL_VERTEX_FORMAT_FLOAT3:			format = VERTEX_FORMAT_FLOAT3;			break;
					case QMODEL_VERTEX_FORMAT_FLOAT4:			format = VERTEX_FORMAT_FLOAT4;			break;
					case QMODEL_VERTEX_FORMAT_INT:				format = VERTEX_FORMAT_INT;				break;
					case QMODEL_VERTEX_FORMAT_INT2:				format = VERTEX_FORMAT_INT2;			break;
					case QMODEL_VERTEX_FORMAT_INT3:				format = VERTEX_FORMAT_INT3;			break;
					case QMODEL_VERTEX_FORMAT_INT4:				format = VERTEX_FORMAT_INT4;			break;
					case QMODEL_VERTEX_FORMAT_UINT:				format = VERTEX_FORMAT_UINT;			break;
					case QMODEL_VERTEX_FORMAT_UINT2:			format = VERTEX_FORMAT_UINT2;			break;
					case QMODEL_VERTEX_FORMAT_UINT3:			format = VERTEX_FORMAT_UINT3;			break;
					case QMODEL_VERTEX_FORMAT_UINT4:			format = VERTEX_FORMAT_UINT4;			break;
					case QMODEL_VERTEX_FORMAT_INT_2_10_10_10:	format = VERTEX_FORMAT_INT_2_10_10_10;	break;
					case QMODEL_VERTEX_FORMAT_UINT_2_10_10_10:	format = VERTEX_FORMAT_UINT_2_10_10_10;	break;
					case QMODEL_VERTEX_FORMAT_FLOAT_10_11_11:	format = VERTEX_FORMAT_FLOAT_10_11_11;	break;
					default: break;
				}

				VertexElement vertexElement;
				vertexElement.attribute = attribute;
				vertexElement.format	= format;

				mesh.elements.PushBack(vertexElement);
			}

			mesh.indexElement.format = INDEX_FORMAT_INVALID;
		
			switch (qModelMesh.indexFormat)
			{
				case QMODEL_INDEX_FORMAT_UINT8:		mesh.indexElement.format = INDEX_FORMAT_UINT8;	break;
				case QMODEL_INDEX_FORMAT_UINT16:	mesh.indexElement.format = INDEX_FORMAT_UINT16;	break;
				case QMODEL_INDEX_FORMAT_UINT32:	mesh.indexElement.format = INDEX_FORMAT_UINT32;	break;
				default: break;
			}

			mFile.SetFilePtr(qModelMesh.verticesOffset, FILE_PTR_BEGIN);

			if (!mFile.Read(vertexData.pVertexBuffer->Data() + vertexBufferOffset, qModelMesh.verticesSizeBytes))
			{
				mpBufferAllocator->Free(vertexData.pVertexBuffer);
				mpBufferAllocator->Free(vertexData.pIndexBuffer);
				mpModelAllocator->Free(pModel);
				return false;
			}

			mFile.SetFilePtr(qModelMesh.indicesOffset, FILE_PTR_BEGIN);

			if (!mFile.Read(vertexData.pIndexBuffer->Data() + indexBufferOffset, qModelMesh.indicesSizeBytes))
			{
				mpBufferAllocator->Free(vertexData.pVertexBuffer);
				mpBufferAllocator->Free(vertexData.pIndexBuffer);
				mpModelAllocator->Free(pModel);
				return false;
			}

			vertexBufferOffset += qModelMesh.verticesSizeBytes;
			indexBufferOffset += qModelMesh.indicesSizeBytes;
		}

		mpModel = pModel;

		return true;
	}

	bool QModelParser::EndReading()
	{
		mFile.Close();

		return true;
	}

	QModelParser::QModelParser(File& qModelFile, 
		PoolAllocator<Model>* pModelAllocator,
		PoolAllocator<ByteBuffer>* pBufferAllocator) :
		mFile(qModelFile), 
		mHeader{},
		mpModelAllocator(pModelAllocator),
		mpBufferAllocator(pBufferAllocator) { }

	void QModelParser::SetModel(const Model& model)
	{
		mpModel = const_cast<Model*>(&model);
	}

	bool QModelParser::Read()
	{
		if (!BeginReading())
		{
			mFile.Close();
			return false;
		}

		if (!ReadStrings())
		{
			mFile.Close();
			return false;
		}

		if (!ReadMeshes())
		{
			mFile.Close();
			return false;
		}

		if(!EndReading())
		{
			mFile.Close();
			return false;
		}

		return true;
	}

	bool QModelParser::Write()
	{
		if (!BeginWriting())
		{
			mFile.Close();
			return false;
		}

		for (const Mesh& mesh : mpModel->meshes)
		{
			if (!WriteMesh(mesh, mpModel->vertexData))
			{
				mFile.Close();
				return false;
			}
		}

		if (!WriteStrings())
		{
			mFile.Close();
			return false;
		}

		if (!EndWriting())
		{
			mFile.Close();
			return false;
		}

		return true;
	}
}