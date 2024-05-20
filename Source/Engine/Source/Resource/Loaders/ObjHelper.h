#pragma once

#include "Resource/Assets/ObjModel.h"
#include "Resource/Assets/Model.h"

#include "Utility/StringReader.h"
#include "Types/Map.h"

namespace Quartz
{
	inline bool ParseOBJ(const String& data, ObjModel& outObjModel)
	{
		StringReader parser(data);

		// @NOTE: if objects rehashes, the loader breaks
		Map<String, ObjObject> objects(1024);

		ObjObject* pObjObject = &objects.Put("_obj_model_", ObjObject());

		while (!parser.IsEnd())
		{
			parser.SkipWhitespace();

			if (parser.Peek() == '\n' || parser.Peek() == '#')
			{
				parser.ReadLine();
				continue;
			}

			Substring token = parser.ReadTo(" ");

			// Comment
			if (token == "#")
			{
				parser.ReadLine();
				continue;
			}

			// Object
			else if (token == "usemtl")
			{
				parser.SkipWhitespace();
				
				Substring objectName = parser.ReadLine();
				objectName = objectName.TrimWhitespaceReverse();

				if (objectName.Length() > 0)
				{
					String objectNameStr(objectName);

					auto& objectIt = objects.Find(objectNameStr);
					if (objectIt != objects.End())
					{
						pObjObject = &objectIt->value;
					}
					else
					{
						ObjObject newObject;
						newObject.material = objectNameStr;

						pObjObject = &objects.Put(objectNameStr, newObject);
					}
				}

				continue;
			}

			// Vertex position
			else if (token == "v")
			{
				parser.SkipWhitespace();

				float x = parser.ReadFloat();
				float y = parser.ReadFloat();
				float z = parser.ReadFloat();

				outObjModel.positions.PushBack(Vec3f{ x, y, z });

				parser.ReadLine();
			}

			// Vertex normal
			else if (token == "vn")
			{
				parser.SkipWhitespace();

				float x = parser.ReadFloat();
				float y = parser.ReadFloat();
				float z = parser.ReadFloat();

				outObjModel.normals.PushBack(Vec3f{ x, y, z });

				parser.ReadLine();
			}

			// Vertex texture coordinates
			else if (token == "vt")
			{
				parser.SkipWhitespace();

				float x = parser.ReadFloat();
				float y = parser.ReadFloat();

				outObjModel.texCoords.PushBack(Vec2f{ x, y });

				parser.ReadLine();
			}

			// Face
			else if (token == "f")
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

					if (faceLine.Peek() == '/')
					{
						faceLine.Read();

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

						indices.PushBack(index);
						++indexCount;

						faceLine.SkipWhitespace();
					}
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

	inline bool ConvertOBJToModel(const ObjModel& objModel, Model& outModel)
	{
		VertexData& vertexData	= outModel.vertexData;
		ByteBuffer& vertices	= *vertexData.pVertexBuffer;
		ByteBuffer& indices		= *vertexData.pIndexBuffer;

		uInt64 vertexBufferOffset	= 0;
		uInt64 indexBufferOffset	= 0;

		uInt32 idx = 0;

		for (const ObjObject& object : objModel.objects)
		{
			Mesh mesh;

			Map<OBJIndex, uInt32>	indexMap;
			bool					is16Bit;

			mesh.name			= object.material;
			mesh.materialPath	= "/Materials/Path";
			mesh.lod			= 0;

			mesh.elements =
			{
				{ VERTEX_ATTRIBUTE_POSITION, VERTEX_FORMAT_FLOAT3 },
				{ VERTEX_ATTRIBUTE_NORMAL, VERTEX_FORMAT_FLOAT3 }
			};

			if (objModel.maxIndex <= INDEX_MAX_UINT16)
			{
				mesh.indexElement.format = INDEX_FORMAT_UINT16;
				is16Bit = true;
			}
			else
			{
				mesh.indexElement.format = INDEX_FORMAT_UINT32;
				is16Bit = false;
			}

			for (const OBJIndex& index : object.indices)
			{
				auto& idxIt = indexMap.Find(index);
				if (idxIt != indexMap.End())
				{
					if (is16Bit)
					{
						indices.Write<uInt16>((uInt16)idxIt->value);
					}
					else
					{
						indices.Write<uInt32>((uInt32)idxIt->value);
					}
				}
				else
				{
					vertices.Write(objModel.positions[index.posIdx].x);
					vertices.Write(objModel.positions[index.posIdx].y);
					vertices.Write(objModel.positions[index.posIdx].z);

					vertices.Write(objModel.normals[index.normIdx].x);
					vertices.Write(objModel.normals[index.normIdx].y);
					vertices.Write(objModel.normals[index.normIdx].z);

					if (is16Bit)
					{
						indices.Write<uInt16>((uInt16)idx);
					}
					else
					{
						indices.Write<uInt32>((uInt32)idx);
					}

					indexMap.Put(index, idx);

					idx++;
				}
			}

			mesh.verticesStartBytes	= vertexBufferOffset;
			mesh.verticesSizeBytes	= vertices.Size() - vertexBufferOffset;
			mesh.indicesStartBytes	= indexBufferOffset;
			mesh.indicesSizeBytes	= indices.Size() - indexBufferOffset;

			if (is16Bit && indices.Size() % 4 != 0)
			{
				// Write a trailing zero for alignment with 32 bit indices
				indices.Write<uInt16>((uInt16)0);
			}

			outModel.meshes.PushBack(mesh);

			vertexBufferOffset += mesh.verticesSizeBytes;
			indexBufferOffset += mesh.indicesSizeBytes;
		}

		return true;
	}
}