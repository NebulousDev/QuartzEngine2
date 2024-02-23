#include "TerrainRenderer.h"

#include "Math/Math.h"

namespace Quartz
{
	ModelData TerrainRenderer::CreateTerrainChunkMesh(uSize resolution)
	{
		ModelData data;

		data.vertices.Reserve((resolution + 1) * (resolution + 1) * 6);
		data.indices.Reserve(resolution * resolution * 6);

		for (uInt32 y = 0; y < resolution + 1; y++)
		{
			for (uInt32 x = 0; x < resolution + 1; x++)
			{
				// @TODO: optimize
				data.vertices.PushBack((float)x / resolution);
				data.vertices.PushBack(0.0f);
				data.vertices.PushBack((float)y / resolution);

				data.vertices.PushBack((float)x / resolution);
				data.vertices.PushBack(((float)x + (float)y) / 2.0f);
				data.vertices.PushBack((float)y / resolution);
			}
		}

		for (uInt32 y = 0; y < resolution; y++)
		{
			for (uInt32 x = 0; x < resolution; x++)
			{
				uInt32 downLeft		= x + (y + 0) * (resolution + 1);
				uInt32 downRight	= x + (y + 0) * (resolution + 1) + 1;
				uInt32 upLeft		= x + (y + 1) * (resolution + 1);
				uInt32 upRight		= x + (y + 1) * (resolution + 1) + 1;

				// @TODO: optimize
				data.indices.PushBack(downLeft);
				data.indices.PushBack(upLeft);
				data.indices.PushBack(upRight);
				data.indices.PushBack(downLeft);
				data.indices.PushBack(upRight);
				data.indices.PushBack(downRight);
			}
		}

		return data;
	}

	Array<float> TerrainRenderer::CreatePerlinNoiseTexture(uSize resolution, uInt64 seed, const Array<float>& octaveWeights)
	{
		Array<float> finalNoise(resolution * resolution);

		for (float y = 0; y < resolution; y++)
		{
			for (float x = 0; x < resolution; x++)
			{
				float freq		= 1.0f;
				float maxAmp	= 0.0f;
				float value		= 0.0f;

				for (float amplitude : octaveWeights)
				{
					constexpr float gridSize = 500.0f;

					float seedX = (x * freq / gridSize) + seed;
					float seedY = (y * freq / gridSize) + seed;

					value += PerlinNoise2D(seedX, seedY) * amplitude;

					freq *= 2.0f;
					maxAmp += amplitude;
				}

				finalNoise[x + y * resolution] = (value / maxAmp) + 0.5f;
			}
		}

		return finalNoise;
	}
}

