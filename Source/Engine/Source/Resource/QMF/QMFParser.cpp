#include "Resource/QMF/QMFParser.h"

#include "Log.h"

namespace Quartz
{
	QMF QMFParser::WriteBlankQMFHeader()
	{
		QMFLayout layout;
		layout.materialCount		= 0;
		layout.shaderCount			= 0;
		layout.meshCount			= 0;

		QMFStringTable stringTable;
		stringTable.encoding		= QMF_STRING_UTF8;
		stringTable.stringCount		= 0;
		stringTable.strsOffset		= 0;
		stringTable.strsSizeBytes	= 0;

		QMFMeshTable meshTable;
		meshTable.meshCount			= 0;
		meshTable.meshesOffset		= 0;
		meshTable.meshesSizeBytes	= 0;
		meshTable.nextExtOffset		= 0;

		QMF qmf;
		qmf.version					= 0x01;
		qmf.layout					= layout;
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
		else if (!mFile.Open(FILE_OPEN_WRITE | FILE_OPEN_BINARY | FILE_OPEN_CLEAR))
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

	bool QMFParser::WriteMesh(const Model& model)
	{
		const VertexData& vertexData = model.data;
		QMFMeshTable& meshTable = mHeader.meshTable;

		uInt64 fileOffset = mFile.GetFilePtr();

		if (meshTable.meshesOffset == 0)
		{
			meshTable.meshesOffset = fileOffset;
		}

		QMFMesh mesh;
		mesh.meshNameID			= RegisterString("TEMP_MODEL_STR");
		mesh.indexFormat		= (QMFIndexFormat)vertexData.index.FormatID();
		mesh.elementCount		= vertexData.elements.Size();
		mesh.elementOffset		= fileOffset + sizeof(QMFMesh);
		mesh.verticesOffset		= mesh.elementOffset + mesh.elementCount * sizeof(QMFMeshElement);
		mesh.verticesSizeBytes	= vertexData.pVertexBuffer->Size();
		mesh.indicesOffset		= mesh.verticesOffset + mesh.verticesSizeBytes;
		mesh.indicesSizeBytes	= vertexData.pIndexBuffer->Size();

		if (!mFile.WriteValues<QMFMesh>(&mesh, 1))
		{
			return false;
		}

		meshTable.meshesSizeBytes += sizeof(QMFMesh);

		for (const VertexElement& vertElement : vertexData.elements)
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

		if (!mFile.Write(vertexData.pVertexBuffer->Data(), mesh.verticesSizeBytes))
		{
			return false;
		}

		if (!mFile.Write(vertexData.pIndexBuffer->Data(), mesh.indicesSizeBytes))
		{
			return false;
		}

		meshTable.meshesSizeBytes += mesh.verticesSizeBytes + mesh.indicesSizeBytes;
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
		else if (!mFile.Open(FILE_OPEN_READ | FILE_OPEN_BINARY))
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

		for (uSize i = 0; i < mHeader.meshTable.meshCount; i++)
		{
			Model* pModel = mpModelAllocator->Allocate(&mFile);
			VertexData& vertexData = pModel->data;

			QMFMesh mesh;

			if (!mFile.ReadValues<QMFMesh>(&mesh, 1))
			{
				mpModelAllocator->Free(pModel);
				return false;
			}

			mFile.SetFilePtr(mesh.elementOffset, FILE_PTR_BEGIN);

			Array<QMFMeshElement, 8> meshElements(mesh.elementCount);

			// @TODO: check mesh.elementCount < 8

			if (!mFile.ReadValues<QMFMeshElement>(meshElements.Data(), mesh.elementCount))
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

				vertexData.elements.PushBack(vertexElement);
			}

			vertexData.index.format = INDEX_FORMAT_INVALID;

			switch (mesh.indexFormat)
			{
				case QMF_INDEX_FORMAT_UINT8:	vertexData.index.format = INDEX_FORMAT_UINT8;	break;
				case QMF_INDEX_FORMAT_UINT16:	vertexData.index.format = INDEX_FORMAT_UINT16;	break;
				case QMF_INDEX_FORMAT_UINT32:	vertexData.index.format = INDEX_FORMAT_UINT32;	break;
				default: break;
			}

			const uInt64 verticesSizeBytes	= mesh.verticesSizeBytes;
			const uInt64 indicesSizeBytes	= mesh.indicesSizeBytes;

			vertexData.pVertexBuffer = mpBufferAllocator->Allocate(verticesSizeBytes);
			vertexData.pIndexBuffer = mpBufferAllocator->Allocate(indicesSizeBytes);

			vertexData.pVertexBuffer->Allocate(verticesSizeBytes);
			vertexData.pIndexBuffer->Allocate(indicesSizeBytes);

			mFile.SetFilePtr(mesh.verticesOffset, FILE_PTR_BEGIN);

			if (!mFile.Read(vertexData.pVertexBuffer->Data(), verticesSizeBytes))
			{
				mpBufferAllocator->Free(vertexData.pVertexBuffer);
				mpBufferAllocator->Free(vertexData.pIndexBuffer);
				mpModelAllocator->Free(pModel);
				return false;
			}

			mFile.SetFilePtr(mesh.indicesOffset, FILE_PTR_BEGIN);

			if (!mFile.Read(vertexData.pIndexBuffer->Data(), indicesSizeBytes))
			{
				mpBufferAllocator->Free(vertexData.pVertexBuffer);
				mpBufferAllocator->Free(vertexData.pIndexBuffer);
				mpModelAllocator->Free(pModel);
				return false;
			}

			mModels.PushBack(pModel);
		}

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

	void QMFParser::AddMesh(const Model& model)
	{
		mModels.PushBack(const_cast<Model*>(&model));
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

		for (const Model* pModel : mModels)
		{
			if (!WriteMesh(*pModel))
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