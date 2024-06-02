#pragma once

#include "Resource/Common.h"
#include "Resource/Assets/ObjModel.h"
#include "Resource/Assets/Model.h"

#include "Utility/StringReader.h"
#include "Types/Map.h"

namespace Quartz
{
	struct ObjConvertSettings
	{
		bool writePositions;
		bool writeNormals;
		bool writeTangents;
		bool writeBiTangents;
		bool writeTexCoords;

		uInt32 positionStreamIdx;
		uInt32 normalStreamIdx;
		uInt32 tangentStreamIdx;
		uInt32 biTangentStreamIdx;
		uInt32 texCoordStreamIdx;

		VertexFormat positionFormat;
		VertexFormat normalFormat;
		VertexFormat tangentFormat;
		VertexFormat biTangentFormat;
		VertexFormat texCoordFormat;

		bool useTangentBiTanW;
		bool flipNormals;
	};

	inline bool ParseOBJ(const String& data, ObjModel& outObjModel)
	{
		StringReader parser(data);

		// @NOTE: if objects rehashes, the loader breaks
		Map<Substring, ObjObject> objects(1024);
		uSize nextMaterialIdx = 0;

		ObjObject* pObjObject = &objects.Put(("_obj_model_"_STR).Substring(0), ObjObject());

		while (!parser.IsEnd())
		{
			parser.SkipWhitespace();

			if (parser.Peek() == '\n' || parser.Peek() == '#')
			{
				parser.ReadLine();
				continue;
			}

			Substring token = parser.ReadThrough(" "_WRAP);

			// Comment
			if (token.StartsWith("#"_WRAP))
			{
				parser.ReadLine();
				continue;
			}

			// Object
			else if (token == "usemtl"_WRAP)
			{
				parser.SkipWhitespace();
				
				Substring objectName = parser.ReadLine();
				objectName = objectName.TrimWhitespaceReverse();

				if (objectName.Length() > 0)
				{
					auto& objectIt = objects.Find(objectName);
					if (objectIt != objects.End())
					{
						pObjObject = &objectIt->value;
					}
					else
					{
						ObjObject newObject;
						newObject.materialName = objectName;
						newObject.materialIdx = nextMaterialIdx++;

						pObjObject = &objects.Put(objectName, newObject);
					}
				}

				continue;
			}

			// Vertex position
			else if (token == "v"_WRAP)
			{
				parser.SkipWhitespace();

				float x = parser.ReadFloat();
				float y = parser.ReadFloat();
				float z = parser.ReadFloat();

				outObjModel.positions.PushBack(Vec3f{ x, y, z });

				parser.ReadLine();
			}

			// Vertex normal
			else if (token == "vn"_WRAP)
			{
				parser.SkipWhitespace();

				float x = parser.ReadFloat();
				float y = parser.ReadFloat();
				float z = parser.ReadFloat();

				outObjModel.normals.PushBack(Vec3f{ x, y, z });

				parser.ReadLine();
			}

			// Vertex texture coordinates
			else if (token == "vt"_WRAP)
			{
				parser.SkipWhitespace();

				float x = parser.ReadFloat();
				float y = parser.ReadFloat();

				outObjModel.texCoords.PushBack(Vec2f{ x, y });

				parser.ReadLine();
			}

			// Face
			else if (token == "f"_WRAP)
			{
				parser.SkipWhitespace();

				StringReader faceLine(parser.ReadLine());

				OBJIndex index{};
				uInt32 indexCount = 0;

				Array<OBJIndex>& indices = pObjObject->indices;

				while (!faceLine.IsEnd())
				{
					if (indexCount > 2)
					{
						OBJIndex index0 = indices[indices.Size() - 3];
						OBJIndex index1 = indices[indices.Size() - 1];
						indices.PushBack(Move(index0));
						indices.PushBack(Move(index1));
						indexCount += 2;
					}

					faceLine.SkipWhitespace();

					index.posIdx = faceLine.ReadInt() - 1;

					faceLine.SkipWhitespace();

					if (faceLine.Peek() == '/')
					{
						faceLine.Read();

						faceLine.SkipWhitespace();

						// No texCoord
						if (faceLine.Peek() == '/')
						{
							faceLine.Read();
							index.normIdx = faceLine.ReadInt() - 1;
						}
						else
						{
							index.texIdx = faceLine.ReadInt() - 1;
						}

						if (faceLine.Peek() == '/')
						{
							faceLine.Read();
							index.normIdx = faceLine.ReadInt() - 1;
						}

						faceLine.SkipWhitespace();
					}

					indices.PushBack(index);
					++indexCount;
				}
			}

			// Everything else
			else
			{
				parser.ReadLine();
				continue;
			}
		}

		for (auto& objectPair : objects)
		{
			const uInt32 indexCount = objectPair.value.indices.Size();

			if (indexCount > 0)
			{
				outObjModel.objects.PushBack(objectPair.value);
				outObjModel.maxIndex += indexCount;
			}
		}

		return true;
	}

	inline bool SetupOrAppendStream(uSize streamIdx, VertexElement& streamElement, Array<VertexStream, 8>& outVertexStreams)
	{
		if (outVertexStreams[streamIdx].vertexElements.Size() > 0)
		{
			outVertexStreams[streamIdx].vertexElements.PushBack(streamElement);
			outVertexStreams[streamIdx].strideBytes += streamElement.sizeBytes;
		}
		else
		{
			VertexStream vertexStream = { streamIdx, { streamElement }, nullptr, streamElement.sizeBytes };
			outVertexStreams[streamIdx] = vertexStream;
			streamIdx++;
		}

		return true;
	}

	inline bool CreateStreams(const ObjConvertSettings& settings, Array<VertexStream, 8>& outVertexStreams)
	{
		Array<uInt32, 8> offsetBytes(8, 0);

		outVertexStreams.Resize(8);

		if (settings.writePositions)
		{
			VertexElement positionElement;
			positionElement.attribute		= VERTEX_ATTRIBUTE_POSITION;
			positionElement.format			= settings.positionFormat;
			positionElement.offsetBytes		= offsetBytes[settings.positionStreamIdx];
			positionElement.sizeBytes		= VertexFormatSizeBytes(positionElement.format);

			SetupOrAppendStream(settings.positionStreamIdx, positionElement, outVertexStreams);

			offsetBytes[settings.positionStreamIdx] += positionElement.sizeBytes;
		}

		if (settings.writeNormals)
		{
			VertexElement normalElement;
			normalElement.attribute			= VERTEX_ATTRIBUTE_NORMAL;
			normalElement.format			= settings.normalFormat;
			normalElement.offsetBytes		= offsetBytes[settings.normalStreamIdx];
			normalElement.sizeBytes			= VertexFormatSizeBytes(normalElement.format);

			SetupOrAppendStream(settings.normalStreamIdx, normalElement, outVertexStreams);

			offsetBytes[settings.normalStreamIdx] += normalElement.sizeBytes;
		}

		if (settings.writeTangents)
		{
			VertexElement tangentElement;
			tangentElement.attribute		= VERTEX_ATTRIBUTE_TANGENT;
			tangentElement.format			= settings.tangentFormat;
			tangentElement.offsetBytes		= offsetBytes[settings.tangentStreamIdx];
			tangentElement.sizeBytes		= VertexFormatSizeBytes(tangentElement.format);

			SetupOrAppendStream(settings.tangentStreamIdx, tangentElement, outVertexStreams);

			offsetBytes[settings.tangentStreamIdx] += tangentElement.sizeBytes;
		}

		if (settings.writeBiTangents)
		{
			VertexElement biTangentElement;
			biTangentElement.attribute		= VERTEX_ATTRIBUTE_BITANGENT;
			biTangentElement.format			= settings.biTangentFormat;
			biTangentElement.offsetBytes	= offsetBytes[settings.biTangentStreamIdx];
			biTangentElement.sizeBytes		= VertexFormatSizeBytes(biTangentElement.format);

			SetupOrAppendStream(settings.biTangentStreamIdx, biTangentElement, outVertexStreams);

			offsetBytes[settings.biTangentStreamIdx] += biTangentElement.sizeBytes;
		}

		if (settings.writeTexCoords)
		{
			VertexElement texCoordElement;
			texCoordElement.attribute		= VERTEX_ATTRIBUTE_TEXCOORD;
			texCoordElement.format			= settings.texCoordFormat;
			texCoordElement.offsetBytes		= offsetBytes[settings.texCoordStreamIdx];
			texCoordElement.sizeBytes		= VertexFormatSizeBytes(texCoordElement.format);

			SetupOrAppendStream(settings.texCoordStreamIdx, texCoordElement, outVertexStreams);

			offsetBytes[settings.texCoordStreamIdx] += texCoordElement.sizeBytes;
		}

		return true;
	}

	inline bool AllocateVertexStreamBuffers(Array<VertexStream, 8>& vertexStreams, uSize maxIndex, PoolAllocator<ByteBuffer>& bufferAllocator)
	{
		for (uSize i = 0; i < vertexStreams.Size(); i++)
		{
			if (vertexStreams[i].vertexElements.Size() > 0)
			{
				const uSize bufferSizeBytes = maxIndex * vertexStreams[i].strideBytes;

				ByteBuffer* pByteBuffer = bufferAllocator.Allocate(bufferSizeBytes);

				if (!pByteBuffer)
				{
					return false;
				}

				vertexStreams[i].pVertexBuffer = pByteBuffer;
			}
		}

		return true;
	}

	inline bool AllocateIndexStreamBuffer(IndexStream& indexStream, uSize indexCount, uSize indexSizeBytes, PoolAllocator<ByteBuffer>& bufferAllocator)
	{
		const uSize bufferSizeBytes = indexCount * indexSizeBytes;

		ByteBuffer* pByteBuffer = bufferAllocator.Allocate(bufferSizeBytes);

		if (!pByteBuffer)
		{
			return false;
		}

		indexStream.pIndexBuffer = pByteBuffer;

		return true;
	}

	// Returns true if index was found, false otherwise
	inline bool WriteIndex(const OBJIndex& objIndex, Map<OBJIndex, uInt32>& indexMap, 
		uInt32& index, ByteBuffer& indices, bool is16Bit)
	{
		auto& idxIt0 = indexMap.Find(objIndex);
		if (idxIt0 != indexMap.End())
		{
			if (is16Bit)
			{
				indices.Write<uInt16>((uInt16)idxIt0->value);
			}
			else
			{
				indices.Write<uInt32>((uInt32)idxIt0->value);
			}

			return true;
		}
		else
		{
			if (is16Bit)
			{
				indices.Write<uInt16>((uInt16)index);
			}
			else
			{
				indices.Write<uInt32>((uInt32)index);
			}

			indexMap.Put(objIndex, index);
			index++;

			return false;
		}
	}

	inline bool WritePosition(bool shouldWrite, uInt32 objPosIdx, const Array<Vec3f>& objPositions, ByteBuffer& vertices)
	{
		if (shouldWrite)
		{
			return vertices.Write<Vec3f>(objPositions[objPosIdx]);
		}

		return true;
	}

	inline bool WriteNormal(bool shouldWrite, uInt32 objNormIdx, const Array<Vec3f>& objNormals, ByteBuffer& normals,
		const Array<Vec3f>& positions, uInt32 posIdx0, uInt32 posIdx1, uInt32 posIdx2, bool flipNormals)
	{
		if (shouldWrite)
		{
			if (objNormIdx < objNormals.Size())
			{
				Vec3f normal = objNormals[objNormIdx];

				if (flipNormals)
				{
					normal *= -1;
				}

				return normals.Write<Vec3f>(objNormals[objNormIdx]);
			}
			else
			{
				const Vec3f& pos0 = positions[posIdx0];
				const Vec3f& pos1 = positions[posIdx1];
				const Vec3f& pos2 = positions[posIdx2];

				const Vec3f dir0 = pos1 - pos0;
				const Vec3f dir1 = pos2 - pos0;

				Vec3f normal = Cross(dir0, dir1).Normalized();

				if (flipNormals)
				{
					normal *= -1;
				}

				return normals.Write<Vec3f>(normal);
			}
		}
		
		return true;
	}

	inline bool WriteTangent(bool shouldWrite, ByteBuffer& tangents, 
		const Array<Vec2f>& normals, const Array<Vec2f>& texCoords, 
		const OBJIndex& objIndex0, const OBJIndex& objIndex1, const OBJIndex& objIndex2, bool writeBiTanW)
	{
		if (shouldWrite)
		{
			return tangents.Write<Vec3f>(Vec3f(0, 0, 0));
		}
		
		return true;
	}

	inline bool WriteTexCoord(bool shouldWrite, uInt32 objTexIdx, const Array<Vec2f>& objTexCoords, ByteBuffer& texCoords)
	{
		if (shouldWrite)
		{
			if (objTexIdx < objTexCoords.Size())
			{
				return texCoords.Write<Vec2f>(objTexCoords[objTexIdx]);
			}
			else
			{
				return texCoords.Write<Vec2f>(Vec2f(0, 0));
			}
		}

		return true;
	}

	inline bool ConvertOBJToModel(const ObjConvertSettings& settings, 
		const ObjModel& objModel, Model& outModel, PoolAllocator<ByteBuffer>& bufferAllocator)
	{
		CreateStreams(settings, outModel.vertexStreams);

		bool is16Bit = false;

		if (objModel.maxIndex <= UINT16_MAX)
		{
			outModel.indexStream.indexElement.format	= INDEX_FORMAT_UINT16;
			outModel.indexStream.indexElement.sizeBytes = IndexFormatSizeBytes(INDEX_FORMAT_UINT16);
			is16Bit = true;
		}
		else
		{
			outModel.indexStream.indexElement.format	= INDEX_FORMAT_UINT32;
			outModel.indexStream.indexElement.sizeBytes = IndexFormatSizeBytes(INDEX_FORMAT_UINT32);
			is16Bit = false;
		}
		
		if (!AllocateVertexStreamBuffers(outModel.vertexStreams, objModel.maxIndex, bufferAllocator) ||
			!AllocateIndexStreamBuffer(outModel.indexStream, objModel.maxIndex, 
				outModel.indexStream.indexElement.sizeBytes, bufferAllocator))
		{
			// Free previous buffers
			for (VertexStream& stream : outModel.vertexStreams)
			{
				if (stream.pVertexBuffer)
				{
					bufferAllocator.Free(stream.pVertexBuffer);
					stream.pVertexBuffer = nullptr;
				}
			}

			return false;
		}

		ByteBuffer& positions	= *outModel.vertexStreams[settings.positionStreamIdx].pVertexBuffer;
		ByteBuffer& normals		= *outModel.vertexStreams[settings.normalStreamIdx].pVertexBuffer;
		ByteBuffer& tangents	= *outModel.vertexStreams[settings.tangentStreamIdx].pVertexBuffer;
		ByteBuffer& biTangents	= *outModel.vertexStreams[settings.biTangentStreamIdx].pVertexBuffer;
		ByteBuffer& texCoords	= *outModel.vertexStreams[settings.texCoordStreamIdx].pVertexBuffer;

		ByteBuffer& indices		= *outModel.indexStream.pIndexBuffer;

		Map<OBJIndex, uInt32>	indexMap;
		uInt32					index = 0;

		for (const ObjObject& object : objModel.objects)
		{
			Mesh mesh = {};

			const uSize meshIndexStart = indices.Size() / outModel.indexStream.indexElement.sizeBytes;

			mesh.name			= object.materialName;
			mesh.lod			= 0;
			mesh.materialIdx	= object.materialIdx;
			mesh.indexStart		= meshIndexStart;

			for (uInt64 i = 0; i < object.indices.Size(); i += 3)
			{
				const OBJIndex& objIndex0 = object.indices[i + 0];
				const OBJIndex& objIndex1 = object.indices[i + 1];
				const OBJIndex& objIndex2 = object.indices[i + 2];

				if (!WriteIndex(objIndex0, indexMap, index, indices, is16Bit))
				{
					WritePosition(settings.writePositions, objIndex0.posIdx, objModel.positions, positions);
					WriteNormal(settings.writeNormals, objIndex0.normIdx, objModel.normals, normals, 
						objModel.positions, objIndex0.posIdx, objIndex1.posIdx, objIndex2.posIdx, settings.flipNormals);
					//WriteTangent(settings.writeTangents, i, tangents, normals, texCoords, i + 1, i + 2, i + 3, true);
					WriteTexCoord(settings.writeTexCoords, objIndex0.texIdx, objModel.texCoords, texCoords);
				}

				if (!WriteIndex(objIndex1, indexMap, index, indices, is16Bit))
				{
					WritePosition(settings.writePositions, objIndex1.posIdx, objModel.positions, positions);
					WriteNormal(settings.writeNormals, objIndex1.normIdx, objModel.normals, normals,
						objModel.positions, objIndex2.posIdx, objIndex0.posIdx, objIndex1.posIdx, settings.flipNormals);
					//WriteTangent(settings.writeTangents, i + 1, tangents, normals, texCoords, i + 1, i + 2, i + 3, true);
					WriteTexCoord(settings.writeTexCoords, objIndex1.texIdx, objModel.texCoords, texCoords);
				}

				if (!WriteIndex(objIndex2, indexMap, index, indices, is16Bit))
				{
					WritePosition(settings.writePositions, objIndex2.posIdx, objModel.positions, positions);
					WriteNormal(settings.writeNormals, objIndex2.normIdx, objModel.normals, normals,
						objModel.positions, objIndex1.posIdx, objIndex2.posIdx, objIndex0.posIdx, settings.flipNormals);
					WriteTexCoord(settings.writeTexCoords, objIndex2.texIdx, objModel.texCoords, texCoords);
				}
			}

			const uSize meshIndexEnd = indices.Size() / outModel.indexStream.indexElement.sizeBytes;

			mesh.indexCount = meshIndexEnd - meshIndexStart;

			outModel.meshes.PushBack(mesh);
		}

		outModel.indexStream.maxIndex = index;
		outModel.indexStream.indexCount = indices.Size() / outModel.indexStream.indexElement.sizeBytes;

		return true;
	}
}