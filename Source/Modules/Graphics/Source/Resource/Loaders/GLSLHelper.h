#pragma once

#include "Types/Array.h"
#include "Types/Map.h"
#include "Types/String.h"
#include "Utility/StringReader.h"
#include "Resource/Assets/Shader.h"
#include "shaderc/shaderc.hpp"
#include "Log.h"

namespace Quartz
{
#define GLSL_DEBUG_PRINT_UNIFORM_OFFSETS 1

	inline bool LineReadLayoutValue(const Substring& line, const WrapperString& valueName, int64& outValue)
	{
		uSize valueIdx = line.Find(valueName);
		Substring valueStr;

		if (valueIdx != line.Length())
		{
			valueStr = line.Substring(valueIdx);

			valueIdx = valueName.Length();
			valueStr = valueStr.Substring(valueIdx);
			valueIdx = valueStr.Find("=");

			if (valueIdx != line.Length())
			{
				valueIdx += 1;
				valueStr = valueStr.Substring(valueIdx);
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}

		if (valueIdx != line.Length())
		{
			const uSize commaIdx = valueStr.Find(",");
			const uSize parenIdx = valueStr.Find(")");

			const uSize setEndIdx = commaIdx < parenIdx ? commaIdx : parenIdx;

			if (setEndIdx != valueStr.Length())
			{
				valueStr = valueStr.TrimWhitespaceForward();

				StringReader setReader(valueStr);
				outValue = setReader.ReadInt();
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	inline uSize LookupParamTypeSizeBytes(ShaderParamType& paramType)
	{
		static uSize sParamBytes[48]
		{ 
			0, 4, 4, 4, 4, 8, 8, 16, 
			16, 8, 16, 16, 8, 16, 16, 8,
			16, 16, 16, 32, 32, 0, 0, 0,
			0, 0, 0, 0, 0, 256, 0, 0,
			256, 0, 0, 0, 0, 0, 0, 0,
			0, 512, 0, 0, 512, 0, 0, 4
		};

		uSize paramTypeInt = (uSize)paramType;

		if (paramTypeInt > 48)
		{
			return 0;
		}

		return sParamBytes[paramTypeInt];
	}

	inline bool LookupParamTypeInfo(const Substring& typeName, ShaderParamType& outParamType, uSize& outParamSizeBytes)
	{
		// Bound types:

		if      (typeName.Find("sampler") != typeName.Length())		outParamType = SHADER_PARAM_TYPE_SAMPLER;
		else if (typeName.Find("image") != typeName.Length())		outParamType = SHADER_PARAM_TYPE_IMAGE;
		else if (typeName.Find("atomic") != typeName.Length())		outParamType = SHADER_PARAM_TYPE_ATOMIC;

		// Regular types:

		else if (typeName == "bool"_WRAP)		outParamType = SHADER_PARAM_TYPE_BOOL;
		else if (typeName == "int"_WRAP)		outParamType = SHADER_PARAM_TYPE_INT;
		else if (typeName == "uint"_WRAP)		outParamType = SHADER_PARAM_TYPE_UINT;
		else if (typeName == "float"_WRAP)		outParamType = SHADER_PARAM_TYPE_FLOAT;
		else if (typeName == "double"_WRAP)		outParamType = SHADER_PARAM_TYPE_DOUBLE;
		else if (typeName == "vec2"_WRAP)		outParamType = SHADER_PARAM_TYPE_VEC2;
		else if (typeName == "vec3"_WRAP)		outParamType = SHADER_PARAM_TYPE_VEC3;
		else if (typeName == "vec4"_WRAP)		outParamType = SHADER_PARAM_TYPE_VEC4;
		else if (typeName == "mat2"_WRAP)		outParamType = SHADER_PARAM_TYPE_MAT2;
		else if (typeName == "mat3"_WRAP)		outParamType = SHADER_PARAM_TYPE_MAT3;
		else if (typeName == "mat4"_WRAP)		outParamType = SHADER_PARAM_TYPE_MAT4;
		else if (typeName == "dmat2"_WRAP)		outParamType = SHADER_PARAM_TYPE_DMAT2;
		else if (typeName == "dmat3"_WRAP)		outParamType = SHADER_PARAM_TYPE_DMAT3;
		else if (typeName == "dmat4"_WRAP)		outParamType = SHADER_PARAM_TYPE_DMAT4;
		else if (typeName == "bvec2"_WRAP)		outParamType = SHADER_PARAM_TYPE_BVEC2;
		else if (typeName == "bvec3"_WRAP)		outParamType = SHADER_PARAM_TYPE_BVEC3;
		else if (typeName == "bvec4"_WRAP)		outParamType = SHADER_PARAM_TYPE_BVEC4;
		else if (typeName == "ivec2"_WRAP)		outParamType = SHADER_PARAM_TYPE_IVEC2;
		else if (typeName == "ivec3"_WRAP)		outParamType = SHADER_PARAM_TYPE_IVEC3;
		else if (typeName == "ivec4"_WRAP)		outParamType = SHADER_PARAM_TYPE_IVEC4;
		else if (typeName == "uvec2"_WRAP)		outParamType = SHADER_PARAM_TYPE_UVEC2;
		else if (typeName == "uvec3"_WRAP)		outParamType = SHADER_PARAM_TYPE_UVEC3;
		else if (typeName == "uvec4"_WRAP)		outParamType = SHADER_PARAM_TYPE_UVEC4;
		else if (typeName == "dvec2"_WRAP)		outParamType = SHADER_PARAM_TYPE_DVEC2;
		else if (typeName == "dvec3"_WRAP)		outParamType = SHADER_PARAM_TYPE_DVEC3;
		else if (typeName == "dvec4"_WRAP)		outParamType = SHADER_PARAM_TYPE_DVEC4;
		else if (typeName == "mat2x2"_WRAP)		outParamType = SHADER_PARAM_TYPE_MAT2X2;
		else if (typeName == "mat2x3"_WRAP)		outParamType = SHADER_PARAM_TYPE_MAT2X3;
		else if (typeName == "mat2x4"_WRAP)		outParamType = SHADER_PARAM_TYPE_MAT2X4;
		else if (typeName == "mat3x2"_WRAP)		outParamType = SHADER_PARAM_TYPE_MAT3X2;
		else if (typeName == "mat3x3"_WRAP)		outParamType = SHADER_PARAM_TYPE_MAT3X3;
		else if (typeName == "mat3x4"_WRAP)		outParamType = SHADER_PARAM_TYPE_MAT3X4;
		else if (typeName == "mat4x2"_WRAP)		outParamType = SHADER_PARAM_TYPE_MAT4X2;
		else if (typeName == "mat4x3"_WRAP)		outParamType = SHADER_PARAM_TYPE_MAT4X3;
		else if (typeName == "mat4x4"_WRAP)		outParamType = SHADER_PARAM_TYPE_MAT4X4;
		else if (typeName == "dmat2x2"_WRAP)	outParamType = SHADER_PARAM_TYPE_DMAT2X2;
		else if (typeName == "dmat2x3"_WRAP)	outParamType = SHADER_PARAM_TYPE_DMAT2X3;
		else if (typeName == "dmat2x4"_WRAP)	outParamType = SHADER_PARAM_TYPE_DMAT2X4;
		else if (typeName == "dmat3x2"_WRAP)	outParamType = SHADER_PARAM_TYPE_DMAT3X2;
		else if (typeName == "dmat3x3"_WRAP)	outParamType = SHADER_PARAM_TYPE_DMAT3X3;
		else if (typeName == "dmat3x4"_WRAP)	outParamType = SHADER_PARAM_TYPE_DMAT3X4;
		else if (typeName == "dmat4x2"_WRAP)	outParamType = SHADER_PARAM_TYPE_DMAT4X2;
		else if (typeName == "dmat4x3"_WRAP)	outParamType = SHADER_PARAM_TYPE_DMAT4X3;
		else if (typeName == "dmat4x4"_WRAP)	outParamType = SHADER_PARAM_TYPE_DMAT4X4;

		else { return false; }

		outParamSizeBytes = LookupParamTypeSizeBytes(outParamType);

		return true;
	}

	inline bool MultilineReadParams(StringReader& reader, uSize& lineNumber, const Substring& startLine, 
		const Map<WrapperString, Array<ShaderParam, 128>>& structMap, Array<ShaderParam, 128>& outParams, 
		int64 set, int64 binding, bool appendVarName)
	{
		Substring line		= startLine;
		sSize scopeCount	= 0;
		uInt32 offsetBytes	= 0;
		bool isBlock		= false;

		do
		{
			// Look for scopes:

			StringReader bracketOpenReader(line);
			StringReader bracketCloseReader(line);

			while (!bracketOpenReader.IsEmpty())
			{
				bracketOpenReader.ReadTo("{");

				if (!bracketOpenReader.IsEmpty())
				{
					scopeCount++;
					bracketOpenReader.Read();
					isBlock = true;
				}
			}

			while (!bracketCloseReader.IsEmpty())
			{
				bracketCloseReader.ReadTo("}");

				if (!bracketCloseReader.IsEmpty())
				{
					scopeCount--;
					bracketCloseReader.Read();
				}
			}

			// Look for names:

			const uSize simicolonIdx = line.Find(";");
			uSize nameStartIdx = 0;

			if (simicolonIdx != line.Length())
			{
				Substring nameStr = line.Substring(0, simicolonIdx);

				const uSize nameStartSpaceIdx	= line.FindReverse(" ");
				const uSize nameStartBracketIdx = line.FindReverse("}");
				const uSize nameStartTabIdx		= line.FindReverse("\t");

				nameStartIdx = nameStartSpaceIdx > nameStartBracketIdx ? nameStartSpaceIdx : nameStartBracketIdx;
				nameStartIdx = nameStartIdx > nameStartTabIdx ? nameStartIdx : nameStartTabIdx;

				if (nameStartIdx != 0)
				{
					nameStr = nameStr.Substring(nameStartIdx + 1);
				}

				if (scopeCount == 0 && isBlock)
				{
					Substring blockName = nameStr.TrimWhitespace();

					if (!blockName.IsEmpty() && appendVarName)
					{
						for (ShaderParam& param : outParams)
						{
							param.name = blockName + "."_STR + param.name;
						}
					}

					return true;
				}
				else
				{
					nameStr = nameStr.TrimWhitespace();

					// parse array:

					sSize arrayCount = 0;

					// @NOTE: assumes brackets are on the same line, fails otherwise
					const uSize arrayCountOpenIdx = nameStr.FindReverse("[");
					const uSize arrayCountCloseIdx = nameStr.FindReverse("]");

					if (arrayCountOpenIdx != arrayCountCloseIdx)
					{
						Substring countStr = nameStr.Substring(arrayCountOpenIdx + 1, arrayCountCloseIdx);
						countStr = countStr.TrimWhitespace();

						StringReader countReader(countStr);

						arrayCount = countReader.ReadInt();

						if (arrayCount > 1)
						{
							nameStr = nameStr.Substring(0, arrayCountOpenIdx);
						}
					}
					
					// parse type:
					
					Substring typeStr = line.Substring(0, nameStartIdx).TrimWhitespaceReverse();

					const uSize typeStartSpaceIdx	= typeStr.FindReverse(" ");
					const uSize typeStartTabIdx		= typeStr.FindReverse("\t");
					const uSize typeStartIdx		= typeStartSpaceIdx > typeStartTabIdx ? typeStartSpaceIdx : typeStartTabIdx;

					typeStr = typeStr.Substring(typeStartIdx).TrimWhitespace();

					auto& structIt = structMap.Find(typeStr);
					if (structIt != structMap.End())
					{
						for (uSize i = 0; i < arrayCount; i++)
						{
							for (const ShaderParam& structParam : structIt->value)
							{
								ShaderParam param = structParam;

								if (arrayCount > 1)
								{
									// @TODO: Improve with string functions:
									uSize length = snprintf(nullptr, 0, "%s[%d].%s", String(nameStr).Str(), i, String(param.name).Str());
									String arrayedNameStr(length);
									snprintf(arrayedNameStr.Data(), length + 1, "%s[%d].%s", String(nameStr).Str(), i, String(param.name).Str());
									param.name = arrayedNameStr;
								}
								else
								{
									param.name = String(nameStr) + "." + param.name;
								}

								param.set				= set;
								param.binding			= binding;
								param.valueOffsetBytes	= offsetBytes;

								outParams.PushBack(param);

								offsetBytes += param.valueSizeBytes;
							}
						}
					}
					else
					{
						ShaderParamType paramType;
						uSize			paramSize;

						if (!LookupParamTypeInfo(typeStr, paramType, paramSize))
						{
							return false;
						}

						ShaderParam param;
						param.name				= nameStr;
						param.type				= paramType;
						param.set				= set;
						param.binding			= binding;
						param.arrayCount		= arrayCount;
						param.valueOffsetBytes	= offsetBytes;
						param.valueSizeBytes	= paramSize;

						outParams.PushBack(param);

						offsetBytes += param.valueSizeBytes;
					}

					if (scopeCount == 0)
					{
						return true;
					}
				}
			}

			line = reader.ReadLine();
			lineNumber++;
		} 
		while (!reader.IsEmpty());

		return false;
	}

	inline bool AdjustOffsets(Array<ShaderParam>& inOutParams)
	{
		uSize accumulatedOffset = 0;

		for (sSize i = 0; i < ((sSize)inOutParams.Size() - 1); i++)
		{
			ShaderParam& param = inOutParams[i];
			ShaderParam& nextParam = inOutParams[i + 1];

			if (param.type == SHADER_PARAM_TYPE_VEC3 || 
				param.type == SHADER_PARAM_TYPE_IVEC3 || 
				param.type == SHADER_PARAM_TYPE_UVEC3 || 
				param.type == SHADER_PARAM_TYPE_BVEC3)
			{
				if (param.set == nextParam.set && param.binding == nextParam.binding)
				{
					if (nextParam.valueSizeBytes <= 4)
					{
						param.valueSizeBytes -= nextParam.valueSizeBytes;
					}
				}
			}

			param.valueOffsetBytes = accumulatedOffset;

			if ((accumulatedOffset + param.valueSizeBytes) % 16 != 0 && nextParam.valueSizeBytes > 4)
			{
				accumulatedOffset += 4;
			}
			
			accumulatedOffset += param.valueSizeBytes;
		}

		return true;
	}

	inline bool ParseGLSLParams(const String& data, Array<ShaderParam>& outParams)
	{
		StringReader reader(data);
		uSize lineNumber = 1;

		int64 lastSet = 0;
		int64 lastBinding = 0;

		Map<WrapperString, Array<ShaderParam, 128>> structMap;

		while (!reader.IsEmpty())
		{
			reader.SkipWhitespace();

			const Substring line = reader.ReadLine();

			if (line.StartsWith("struct"))
			{
				const Substring structStr = line.Substring(6).TrimWhitespaceForward();

				const uSize nameEndSpaceIdx = structStr.Find(" ");
				const uSize nameEndBracketIdx = structStr.Find("{");
				const uSize nameEndIdx = nameEndSpaceIdx < nameEndBracketIdx ? nameEndSpaceIdx : nameEndBracketIdx;

				const Substring structTypeName = structStr.Substring(0, nameEndIdx);

				Array<ShaderParam, 128> structParams;
				if (!MultilineReadParams(reader, lineNumber, structStr, structMap, structParams, 0, 0, false))
				{
					LogError("Error parsing GLSL text: Malformed line [%d] \"%s\"", lineNumber, line.Str());
					return false;
				}

				structMap.Put(structTypeName, structParams);
			}
			else if (line.StartsWith("layout"))
			{
				const Substring layoutStr = line.Substring(6);

				if (layoutStr.Find("location") != layoutStr.Length() &&
					(layoutStr.Find("in ") != layoutStr.Length() ||
						layoutStr.Find("out ") != layoutStr.Length()))
				{
					// vertex attributes
					continue;
				}

				int64 set		= -1;
				int64 binding	= -1;

				LineReadLayoutValue(layoutStr, "set", set);
				LineReadLayoutValue(layoutStr, "binding", binding);

				if (line.Find("std140") == line.Length())
				{
					LogWarning("GLSL uniform block does not contain 'std140' layout qualifier. Uniform offsets may not be correct!");
				}

				if (set == -1)
				{
					set = lastSet;
				}
				else
				{
					lastSet = set;
				}

				if (binding == -1)
				{
					binding = lastBinding++;
				}
				else
				{
					lastBinding = binding;
				}
				
				Array<ShaderParam, 128> blockParams;
				if (!MultilineReadParams(reader, lineNumber, layoutStr, structMap, blockParams, set, binding, true))
				{
					LogError("Error parsing GLSL text: Malformed line [%d] \"%s\"", lineNumber, line.Str());
					return false;
				}

				for (const ShaderParam& param : blockParams)
				{
					outParams.PushBack(param);
				}
			}

			lineNumber++;
		}

		AdjustOffsets(outParams);

#if GLSL_DEBUG_PRINT_UNIFORM_OFFSETS

		for (ShaderParam& param : outParams)
		{
			LogInfo("%s : size = %d, offset = %d", param.name.Str(), param.valueSizeBytes, param.valueOffsetBytes);
		}

#endif

		return true;
	}

	inline shaderc_shader_kind ShaderStageToShaderKind(ShaderStage shaderStage)
	{
		switch (shaderStage)
		{
			case SHADER_STAGE_VERTEX:					return shaderc_vertex_shader;
			case SHADER_STAGE_TESSELLATION_CONTROL:		return shaderc_tess_control_shader;
			case SHADER_STAGE_TESSELLATION_EVALUATION:	return shaderc_tess_evaluation_shader;
			case SHADER_STAGE_GEOMETRY:					return shaderc_geometry_shader;
			case SHADER_STAGE_FRAGMENT:					return shaderc_fragment_shader;
			case SHADER_STAGE_COMPUTE:					return shaderc_compute_shader;
			case SHADER_STAGE_KERNEL:					return shaderc_glsl_infer_from_source; // <<
			case SHADER_STAGE_TASK:						return shaderc_task_shader;
			case SHADER_STAGE_MESH:						return shaderc_mesh_shader;
			case SHADER_STAGE_RAY_GENERATION:			return shaderc_raygen_shader;
			case SHADER_STAGE_INTERSECTION:				return shaderc_intersection_shader;
			case SHADER_STAGE_ANY_HIT:					return shaderc_anyhit_shader;
			case SHADER_STAGE_CLOSEST_HIT:				return shaderc_closesthit_shader;
			case SHADER_STAGE_MISS:						return shaderc_miss_shader;
			case SHADER_STAGE_CALLABLE:					return shaderc_callable_shader;
		}

		return shaderc_glsl_infer_from_source;
	}

	inline bool CompileGLSL(const String& shaderName, const String& glslText, Array<uInt8>& outSpirv, ShaderStage shaderStage)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		LogInfo("Compiling GLSL Shader '%s'...", shaderName.Str());

		if (shaderStage == SHADER_STAGE_INVALID)
		{
			LogError("Failed to compile GLSL Shader '%s': Invalid shader type.", shaderName.Str());
			return false;
		}

		options.SetOptimizationLevel(shaderc_optimization_level_performance);
		//options.SetOptimizationLevel(shaderc_optimization_level_zero);
		options.SetGenerateDebugInfo();

		shaderc_shader_kind shaderKind = ShaderStageToShaderKind(shaderStage);

		shaderc::SpvCompilationResult module =
			compiler.CompileGlslToSpv(glslText.Str(), shaderKind, shaderName.Str(), options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LogError("Failed to compile GLSL Shader '%s':\n%s", shaderName.Str(), module.GetErrorMessage().c_str());
			return false;
		}

		// @TODO: Improve this. 3 copies is too many
		std::vector<uInt32> binary = { module.cbegin(), module.cend() };
		outSpirv.Resize(binary.size() * sizeof(uInt32));
		memcpy_s(outSpirv.Data(), outSpirv.Size(), binary.data(), binary.size() * sizeof(uInt32));

		return true;
	}
}