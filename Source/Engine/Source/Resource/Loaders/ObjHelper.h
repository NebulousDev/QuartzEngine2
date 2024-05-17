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
			else if (token == "o")
			{
				parser.ReadLine();
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

				Array<OBJIndex>& indices = outObjModel.indices;

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

		return true;
	}

	inline bool ConvertOBJToModel(const ObjModel& objModel, Model& outModel)
	{
		VertexData& vertexData	= outModel.data;
		ByteBuffer& vertices	= *vertexData.pVertexBuffer;
		ByteBuffer& indices		= *vertexData.pIndexBuffer;

		Map<OBJIndex, uInt32>	indexMap;
		bool					is16Bit;

		outModel.data.elements =
		{
			{ VERTEX_ATTRIBUTE_POSITION, VERTEX_FORMAT_FLOAT3 },
			{ VERTEX_ATTRIBUTE_NORMAL, VERTEX_FORMAT_FLOAT3 }
		};

		if (objModel.indices.Size() <= INDEX_MAX_UINT16)
		{
			outModel.data.index.format = INDEX_FORMAT_UINT16;
			is16Bit = true;
		}
		else
		{
			outModel.data.index.format = INDEX_FORMAT_UINT32;
			is16Bit = false;
		}

		uInt32 idx = 0;
		for (const OBJIndex& index : objModel.indices)
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

		return true;
	}
}