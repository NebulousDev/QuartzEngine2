#include "Resource/QMF/QMFParser.h"

#include "Log.h"

namespace Quartz
{
	QMF QMFParser::WriteBlankQMFHeader()
	{

		QMFStringTable stringTable;
		stringTable.encoding		= QMF_STRING_UTF8;
		stringTable._reserved0		= 0;
		stringTable.stringCount		= 0;
		stringTable.strsOffset		= 0;
		stringTable.strsSizeBytes	= 0;
		stringTable.nextExtOffset	= 0;

		QMFMeshTable meshTable;
		meshTable.meshCount				= 0;
		meshTable._reserved0			= 0;
		meshTable.meshesOffset			= 0;
		meshTable.meshesSizeBytes		= 0;
		meshTable.vertexBufferSizeBytes	= 0;
		meshTable.indexBufferSizeBytes	= 0;
		meshTable.nextExtOffset			= 0;

		QMF qmf;
		qmf.versionMajor			= 1;
		qmf.versionMinor			= 2;
		qmf._reserved0				= 0;
		qmf.stringTable				= stringTable;
		qmf.meshTable				= meshTable;
		qmf.nextExtOffset			= 0;

		mFile.WriteValues<QMF>(&qmf, 1);

		return qmf;
	}

	QMFStringID QMFParser::RegisterString(const String& string)
	{
		mStrings.PushBack(string);
		return mStrings.Size() - 1;
	}

	bool QMFParser::BeginWriting()
	{
		if (mFile.IsOpen())
		{
			LogWarning("QMF File [%s] is already open.", mFile.GetPath().Str());
			mFile.SetFilePtr(0, FILE_PTR_BEGIN);
		}
		else if (!mFile.Open(FILE_OPEN_WRITE | FILE_OPEN_BINARY | FILE_OPEN_CLEAR | FILE_OPEN_SHARE_READ))
		{
			LogError("QMF File [%s] failed to open.", mFile.GetPath().Str());
			return false;
		}

		mHeader = WriteBlankQMFHeader();

		return true;
	}

	bool QMFParser::WriteStrings()
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

	bool QMFParser::EndWriting()
	{
		mFile.SetFilePtr(0, FILE_PTR_BEGIN);

		if (!mFile.WriteValues<QMF>(&mHeader, 1))
		{
			return false;
		}

		mFile.Close();

		return true;
	}

	bool QMFParser::WriteMesh(const Mesh& mesh, const VertexData& vertexData)
	{
		QMFMeshTable& meshTable = mHeader.meshTable;

		uInt64 fileOffset = mFile.GetFilePtr();

		if (meshTable.meshesOffset == 0)
		{
			meshTable.meshesOffset = fileOffset;
		}

		meshTable.vertexBufferSizeBytes += mesh.verticesSizeBytes;
		meshTable.indexBufferSizeBytes	+= mesh.indicesSizeBytes;

		QMFMesh qmfMesh;
		qmfMesh.nameID				= RegisterString(mesh.name);
		qmfMesh.materialPathID		= RegisterString(mesh.materialPath);
		qmfMesh.lodIdx				= mesh.lod;
		qmfMesh._reserved0			= 0;
		qmfMesh.indexFormat			= (QMFIndexFormat)mesh.indexElement.FormatID();
		qmfMesh.elementCount		= mesh.elements.Size();
		qmfMesh.elementOffset		= fileOffset + sizeof(QMFMesh);
		qmfMesh.verticesOffset		= qmfMesh.elementOffset + qmfMesh.elementCount * sizeof(QMFMeshElement);
		qmfMesh.verticesSizeBytes	= mesh.verticesSizeBytes;
		qmfMesh.indicesOffset		= qmfMesh.verticesOffset + qmfMesh.verticesSizeBytes;
		qmfMesh.indicesSizeBytes	= mesh.indicesSizeBytes;

		if (!mFile.WriteValues<QMFMesh>(&qmfMesh, 1))
		{
			return false;
		}

		meshTable.meshesSizeBytes += sizeof(QMFMesh);

		for (const VertexElement& vertElement : mesh.elements)
		{
			QMFMeshElement meshElement;
			meshElement.attribute	= (QMFVertexAttribute)vertElement.attribute;
			meshElement.format		= (QMFVertexFormat)vertElement.FormatID();

			if (!mFile.WriteValues<QMFMeshElement>(&meshElement, 1))
			{
				return false;
			}

			meshTable.meshesSizeBytes += sizeof(QMFMeshElement);
		}

		if (!mFile.Write(vertexData.pVertexBuffer->Data() + mesh.verticesStartBytes, mesh.verticesSizeBytes))
		{
			return false;
		}

		if (!mFile.Write(vertexData.pIndexBuffer->Data() + mesh.indicesStartBytes, mesh.indicesSizeBytes))
		{
			return false;
		}

		meshTable.meshesSizeBytes += qmfMesh.verticesSizeBytes + qmfMesh.indicesSizeBytes;
		meshTable.meshCount++;

		return true;
	}

	bool QMFParser::BeginReading()
	{
		if (mFile.IsOpen())
		{
			LogWarning("QMF File [%s] is already open.", mFile.GetPath().Str());
			mFile.SetFilePtr(0, FILE_PTR_BEGIN);
		}
		else if (!mFile.Open(FILE_OPEN_READ | FILE_OPEN_BINARY | FILE_OPEN_SHARE_READ))
		{
			LogError("Error reading QMF File [%s]. Failed to open.", mFile.GetPath().Str());
			return false;
		}

		if (!mFile.ReadValues<QMF>(&mHeader, 1))
		{
			return false;
		}

		if (WrapperString(mHeader.magic, 3) != "QMF"_STR)
		{
			LogError("Error reading QMF File [%s]. File is not a valid QMF file.", mFile.GetPath().Str());
			return false;
		}

		return true;
	}

	bool QMFParser::ReadStrings()
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

	bool QMFParser::ReadMeshes()
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
			QMFMesh qmfMesh;

			if (!mFile.ReadValues<QMFMesh>(&qmfMesh, 1))
			{
				mpBufferAllocator->Free(vertexData.pVertexBuffer);
				mpBufferAllocator->Free(vertexData.pIndexBuffer);
				mpModelAllocator->Free(pModel);
				return false;
			}

			if (qmfMesh.nameID < mStrings.Size())
			{
				mesh.name = mStrings[qmfMesh.nameID];
			}
			else
			{
				mpBufferAllocator->Free(vertexData.pVertexBuffer);
				mpBufferAllocator->Free(vertexData.pIndexBuffer);
				mpModelAllocator->Free(pModel);
				return false;
			}

			if (qmfMesh.materialPathID < mStrings.Size())
			{
				mesh.materialPath = mStrings[qmfMesh.materialPathID];
			}
			else
			{
				mpBufferAllocator->Free(vertexData.pVertexBuffer);
				mpBufferAllocator->Free(vertexData.pIndexBuffer);
				mpModelAllocator->Free(pModel);
				return false;
			}

			mesh.lod				= qmfMesh.lodIdx;
			mesh.verticesStartBytes	= vertexBufferOffset;
			mesh.verticesSizeBytes	= qmfMesh.verticesSizeBytes;
			mesh.indicesStartBytes	= indexBufferOffset;
			mesh.indicesSizeBytes	= qmfMesh.indicesSizeBytes;

			mFile.SetFilePtr(qmfMesh.elementOffset, FILE_PTR_BEGIN);

			Array<QMFMeshElement, 8> meshElements(qmfMesh.elementCount);

			// @TODO: check mesh.elementCount < 8

			if (!mFile.ReadValues<QMFMeshElement>(meshElements.Data(), qmfMesh.elementCount))
			{
				mpModelAllocator->Free(pModel);
				return false;
			}
				
			for (uSize j = 0; j < meshElements.Size(); j++)
			{
				QMFMeshElement& meshElement = meshElements[j];

				VertexAttribute attribute	= VERTEX_ATTRIBUTE_INVALID;
				VertexFormat	format		= VERTEX_FORMAT_INVALID;

				attribute = (VertexAttribute)meshElement.attribute;

				switch (meshElement.format)
				{
					case QMF_VERTEX_FORMAT_FLOAT:			format = VERTEX_FORMAT_FLOAT;			break;
					case QMF_VERTEX_FORMAT_FLOAT2:			format = VERTEX_FORMAT_FLOAT2;			break;
					case QMF_VERTEX_FORMAT_FLOAT3:			format = VERTEX_FORMAT_FLOAT3;			break;
					case QMF_VERTEX_FORMAT_FLOAT4:			format = VERTEX_FORMAT_FLOAT4;			break;
					case QMF_VERTEX_FORMAT_INT:				format = VERTEX_FORMAT_INT;				break;
					case QMF_VERTEX_FORMAT_INT2:			format = VERTEX_FORMAT_INT2;			break;
					case QMF_VERTEX_FORMAT_INT3:			format = VERTEX_FORMAT_INT3;			break;
					case QMF_VERTEX_FORMAT_INT4:			format = VERTEX_FORMAT_INT4;			break;
					case QMF_VERTEX_FORMAT_UINT:			format = VERTEX_FORMAT_UINT;			break;
					case QMF_VERTEX_FORMAT_UINT2:			format = VERTEX_FORMAT_UINT2;			break;
					case QMF_VERTEX_FORMAT_UINT3:			format = VERTEX_FORMAT_UINT3;			break;
					case QMF_VERTEX_FORMAT_UINT4:			format = VERTEX_FORMAT_UINT4;			break;
					case QMF_VERTEX_FORMAT_INT_2_10_10_10:	format = VERTEX_FORMAT_INT_2_10_10_10;	break;
					case QMF_VERTEX_FORMAT_UINT_2_10_10_10:	format = VERTEX_FORMAT_UINT_2_10_10_10;	break;
					case QMF_VERTEX_FORMAT_FLOAT_10_11_11:	format = VERTEX_FORMAT_FLOAT_10_11_11;	break;
					default: break;
				}

				VertexElement vertexElement;
				vertexElement.attribute = attribute;
				vertexElement.format	= format;

				mesh.elements.PushBack(vertexElement);
			}

			mesh.indexElement.format = INDEX_FORMAT_INVALID;
		
			switch (qmfMesh.indexFormat)
			{
				case QMF_INDEX_FORMAT_UINT8:	mesh.indexElement.format = INDEX_FORMAT_UINT8;	break;
				case QMF_INDEX_FORMAT_UINT16:	mesh.indexElement.format = INDEX_FORMAT_UINT16;	break;
				case QMF_INDEX_FORMAT_UINT32:	mesh.indexElement.format = INDEX_FORMAT_UINT32;	break;
				default: break;
			}

			mFile.SetFilePtr(qmfMesh.verticesOffset, FILE_PTR_BEGIN);

			if (!mFile.Read(vertexData.pVertexBuffer->Data() + vertexBufferOffset, qmfMesh.verticesSizeBytes))
			{
				mpBufferAllocator->Free(vertexData.pVertexBuffer);
				mpBufferAllocator->Free(vertexData.pIndexBuffer);
				mpModelAllocator->Free(pModel);
				return false;
			}

			mFile.SetFilePtr(qmfMesh.indicesOffset, FILE_PTR_BEGIN);

			if (!mFile.Read(vertexData.pIndexBuffer->Data() + indexBufferOffset, qmfMesh.indicesSizeBytes))
			{
				mpBufferAllocator->Free(vertexData.pVertexBuffer);
				mpBufferAllocator->Free(vertexData.pIndexBuffer);
				mpModelAllocator->Free(pModel);
				return false;
			}

			vertexBufferOffset += qmfMesh.verticesSizeBytes;
			indexBufferOffset += qmfMesh.indicesSizeBytes;
		}

		mpModel = pModel;

		return true;
	}

	bool QMFParser::EndReading()
	{
		mFile.Close();

		return true;
	}

	QMFParser::QMFParser(File& qmfFile, 
		PoolAllocator<Model>* pModelAllocator,
		PoolAllocator<ByteBuffer>* pBufferAllocator) :
		mFile(qmfFile), 
		mHeader{},
		mpModelAllocator(pModelAllocator),
		mpBufferAllocator(pBufferAllocator) { }

	void QMFParser::SetModel(const Model& model)
	{
		mpModel = const_cast<Model*>(&model);
	}

	bool QMFParser::Read()
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

	bool QMFParser::Write()
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