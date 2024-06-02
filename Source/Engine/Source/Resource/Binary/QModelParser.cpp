#include "Resource/Binary/QModelParser.h"

#include "Log.h"

namespace Quartz
{
	QModel QModelParser::WriteBlankQModelHeader()
	{
		QStringTable stringTable;
		stringTable.encoding			= STRING_ENCODING_UTF8;
		stringTable._reserved0			= 0;
		stringTable.stringCount			= 0;
		stringTable.strsOffset			= 0;
		stringTable.strsSizeBytes		= 0;
		stringTable.nextExtOffset		= 0;

		QModelMeshTable meshTable;
		meshTable.meshCount				= 0;
		meshTable._reserved0			= 0;
		meshTable.meshesOffset			= 0;
		meshTable.meshesSizeBytes		= 0;
		meshTable.nextExtOffset			= 0;

		QModelStreamTable streamTable;
		streamTable.streamCount				= 0;
		streamTable._reserved0				= 0;
		streamTable.vertexStreamsOffset		= 0;
		streamTable.vertexStreamsSizeBytes	= 0;
		streamTable.indexStreamOffset		= 0;
		streamTable.indexStreamSizeBytes	= 0;
		streamTable.nextExtOffset			= 0;

		QModel qModel;					// Magic assigned in header
		qModel.versionMajor				= 1;
		qModel.versionMinor				= 4;
		qModel._reserved0				= 0;
		qModel.stringTable				= stringTable;
		qModel.meshTable				= meshTable;
		qModel.streamTable				= streamTable;
		qModel.nextExtOffset			= 0;

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

	bool QModelParser::WriteMesh(const Mesh& mesh)
	{
		QModelMeshTable& meshTable = mHeader.meshTable;

		uInt64 fileOffset = mFile.GetFilePtr();

		if (meshTable.meshesOffset == 0)
		{
			meshTable.meshesOffset = fileOffset;
		}

		QModelMesh qModelMesh;
		qModelMesh.nameID		= RegisterString(mesh.name);
		qModelMesh.materialIdx	= mesh.materialIdx;
		qModelMesh.lodIdx		= mesh.lod;
		qModelMesh._reserved0	= 0;
		qModelMesh._reserved1	= 0;
		qModelMesh.indexStart	= mesh.indexStart;
		qModelMesh.indexCount	= mesh.indexCount;

		if (!mFile.WriteValues<QModelMesh>(&qModelMesh, 1))
		{
			return false;
		}

		meshTable.meshesSizeBytes += sizeof(QModelMesh);
		meshTable.meshCount++;

		return true;
	}

	bool QModelParser::WriteVertexStream(const VertexStream& stream)
	{
		QModelStreamTable& streamTable = mHeader.streamTable;

		if (!stream.pVertexBuffer)
		{
			// No data for this streamIdx
			return true;
		}

		uInt64 fileOffset = mFile.GetFilePtr();

		if (streamTable.vertexStreamsOffset == 0)
		{
			streamTable.vertexStreamsOffset = fileOffset;
		}

		QModelVertexElementTable qVertexElementTable;
		qVertexElementTable.elementCount		= stream.vertexElements.Size();
		qVertexElementTable._reserved0			= 0;
		qVertexElementTable.elementsOffset		= fileOffset + sizeof(QModelVertexStream);
		qVertexElementTable.elementsSizeBytes	= qVertexElementTable.elementCount * sizeof(QModelVertexElement);

		QModelVertexStream qVertexStream;
		qVertexStream.streamIdx			= stream.streamIdx;
		qVertexStream._reserved0		= 0;
		qVertexStream.strideBytes		= stream.strideBytes;
		qVertexStream.elementTable		= qVertexElementTable;
		qVertexStream.streamOffset		= qVertexElementTable.elementsOffset + qVertexElementTable.elementsSizeBytes;
 		qVertexStream.streamSizeBytes	= stream.pVertexBuffer->Size();

		streamTable.vertexStreamsSizeBytes += sizeof(QModelVertexStream);

		if (!mFile.WriteValues<QModelVertexStream>(&qVertexStream, 1))
		{
			return false;
		}

		for (const VertexElement& vertexElement : stream.vertexElements)
		{
			QModelVertexElement qVertexElement;
			qVertexElement.attribute	= vertexElement.attribute;
			qVertexElement.format		= vertexElement.format;
			qVertexElement._reserved0	= 0;
			qVertexElement.offsetBytes	= vertexElement.offsetBytes;
			qVertexElement.sizeBytes	= vertexElement.sizeBytes;

			if (!mFile.WriteValues<QModelVertexElement>(&qVertexElement, 1))
			{
				return false;
			}
		}

		if (!mFile.Write(stream.pVertexBuffer->Data(), stream.pVertexBuffer->Size()))
		{
			return false;
		}

		streamTable.vertexStreamsSizeBytes += stream.pVertexBuffer->Size();

		mHeader.streamTable.streamCount++;

		return true;
	}

	bool QModelParser::WriteIndexStream(const IndexStream& stream)
	{
		QModelStreamTable& streamTable = mHeader.streamTable;

		uInt64 fileOffset = mFile.GetFilePtr();

		if (streamTable.indexStreamOffset == 0)
		{
			streamTable.indexStreamOffset = fileOffset;
		}

		QModelIndexElement qIndexElement;
		qIndexElement.format		= stream.indexElement.format;
		qIndexElement._reserved0	= 0;
		qIndexElement.sizeBytes		= stream.indexElement.sizeBytes;
		qIndexElement._reserved1	= 0;

		QModelIndexStream qIndexStream;
		qIndexStream.indexElement		= qIndexElement;
		qIndexStream.indexCount			= stream.indexCount;
		qIndexStream.maxIndex			= stream.maxIndex;
		qIndexStream.streamOffset		= fileOffset + sizeof(QModelIndexStream);
		qIndexStream.streamSizeBytes	= stream.pIndexBuffer->Size();

		streamTable.indexStreamSizeBytes += sizeof(QModelIndexStream);

		if (!mFile.WriteValues<QModelIndexStream>(&qIndexStream, 1))
		{
			return false;
		}

		if (!mFile.Write(stream.pIndexBuffer->Data(), stream.pIndexBuffer->Size()))
		{
			return false;
		}

		streamTable.indexStreamSizeBytes += stream.pIndexBuffer->Size();

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

		pModel->meshes.Resize(mHeader.meshTable.meshCount);

		for (uSize i = 0; i < mHeader.meshTable.meshCount; i++)
		{
			Mesh& mesh = pModel->meshes[i];
			QModelMesh qModelMesh;

			if (!mFile.ReadValues<QModelMesh>(&qModelMesh, 1))
			{
				mpModelAllocator->Free(pModel);
				return false;
			}

			if (qModelMesh.nameID < mStrings.Size())
			{
				mesh.name = mStrings[qModelMesh.nameID];
			}
			else
			{
				mpModelAllocator->Free(pModel);
				return false;
			}

			mesh.materialIdx		= qModelMesh.materialIdx;
			mesh.lod				= qModelMesh.lodIdx;
			mesh.indexStart			= qModelMesh.indexStart;
			mesh.indexCount			= qModelMesh.indexCount;
		}

		mpModel = pModel;

		return true;
	}

	bool QModelParser::ReadVertexStreams()
	{
		if (mHeader.streamTable.streamCount == 0)
		{
			return true;
		}

		mpModel->vertexStreams.Resize(8);

		mFile.SetFilePtr(mHeader.streamTable.vertexStreamsOffset, FILE_PTR_BEGIN);

		for (uSize i = 0; i < mHeader.streamTable.streamCount; i++)
		{
			QModelVertexStream qModelVertexStream;

			if (!mFile.ReadValues<QModelVertexStream>(&qModelVertexStream, 1))
			{
				return false;
			}

			if (qModelVertexStream.streamIdx > mpModel->vertexStreams.Size())
			{
				return false;
			}

			if (qModelVertexStream.elementTable.elementCount > 8)
			{
				return false;
			}

			VertexStream& vertexStream	= mpModel->vertexStreams[qModelVertexStream.streamIdx];
			vertexStream.streamIdx		= qModelVertexStream.streamIdx;
			vertexStream.strideBytes	= qModelVertexStream.strideBytes;

			vertexStream.vertexElements.Resize(qModelVertexStream.elementTable.elementCount);

			mFile.SetFilePtr(qModelVertexStream.elementTable.elementsOffset, FILE_PTR_BEGIN);

			for (uSize j = 0; j < qModelVertexStream.elementTable.elementCount; j++)
			{
				QModelVertexElement qVertexElement;

				if (!mFile.ReadValues<QModelVertexElement>(&qVertexElement, 1))
				{
					return false;
				}

				VertexElement& vertexElement	= vertexStream.vertexElements[j];
				vertexElement.attribute			= qVertexElement.attribute;
				vertexElement.offsetBytes		= qVertexElement.offsetBytes;
				vertexElement.sizeBytes			= qVertexElement.sizeBytes;
				vertexElement.format			= qVertexElement.format;
			}

			vertexStream.pVertexBuffer = mpBufferAllocator->Allocate(qModelVertexStream.streamSizeBytes);

			if (!vertexStream.pVertexBuffer)
			{
				return false;
			}

			vertexStream.pVertexBuffer->Allocate(qModelVertexStream.streamSizeBytes);

			mFile.SetFilePtr(qModelVertexStream.streamOffset, FILE_PTR_BEGIN);

			if (!mFile.Read(vertexStream.pVertexBuffer->Data(), qModelVertexStream.streamSizeBytes))
			{
				// @TODO: free other buffers
				return false;
			}
		}
		
		return true;
	}

	bool QModelParser::ReadIndexStream()
	{
		if (mHeader.streamTable.indexStreamSizeBytes == 0)
		{
			return true;
		}

		mFile.SetFilePtr(mHeader.streamTable.indexStreamOffset, FILE_PTR_BEGIN);

		QModelIndexStream qModelIndexStream;

		if (!mFile.ReadValues<QModelIndexStream>(&qModelIndexStream, 1))
		{
			return false;
		}

		IndexStream& indexStream			= mpModel->indexStream;
		indexStream.indexCount				= qModelIndexStream.indexCount;
		indexStream.maxIndex				= qModelIndexStream.maxIndex;
		indexStream.indexElement.format		= qModelIndexStream.indexElement.format;
		indexStream.indexElement.sizeBytes	= qModelIndexStream.indexElement.sizeBytes;

		indexStream.pIndexBuffer = mpBufferAllocator->Allocate(qModelIndexStream.streamSizeBytes);

		if (!indexStream.pIndexBuffer)
		{
			return false;
		}

		indexStream.pIndexBuffer->Allocate(qModelIndexStream.streamSizeBytes);

		mFile.SetFilePtr(qModelIndexStream.streamOffset, FILE_PTR_BEGIN);

		if (!mFile.Read(indexStream.pIndexBuffer->Data(), qModelIndexStream.streamSizeBytes))
		{
			mpBufferAllocator->Free(indexStream.pIndexBuffer);
			return false;
		}
		
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
			mpModelAllocator->Free(mpModel);
			mFile.Close();
			return false;
		}

		if (!ReadVertexStreams())
		{
			for (VertexStream& stream : mpModel->vertexStreams)
			{
				if (stream.pVertexBuffer)
				{
					mpBufferAllocator->Free(stream.pVertexBuffer);
				}
			}

			mpModelAllocator->Free(mpModel);
			mFile.Close();
			return false;
		}

		if (!ReadIndexStream())
		{
			for (VertexStream& stream : mpModel->vertexStreams)
			{
				if (stream.pVertexBuffer)
				{
					mpBufferAllocator->Free(stream.pVertexBuffer);
				}
			}

			mpModelAllocator->Free(mpModel);
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
		if (!mpModel)
		{
			return false;
		}

		if (!BeginWriting())
		{
			mFile.Close();
			return false;
		}

		for (const Mesh& mesh : mpModel->meshes)
		{
			if (!WriteMesh(mesh))
			{
				mFile.Close();
				return false;
			}
		}

		for (const VertexStream& vertexStream : mpModel->vertexStreams)
		{
			if (!WriteVertexStream(vertexStream))
			{
				mFile.Close();
				return false;
			}
		}

		if (!WriteIndexStream(mpModel->indexStream))
		{
			mFile.Close();
			return false;
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