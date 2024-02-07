#pragma once

#include <vulkan/vulkan.h>
#include <spirv-headers/spirv.h>

#include "Types/Array.h"
#include "Types/String.h"

namespace Quartz
{
	enum SpirvType
	{
		SPIRV_TYPE_UNKNOWN,
		SPIRV_TYPE_VOID,
		SPIRV_TYPE_BOOL,
		SPIRV_TYPE_INT,
		SPIRV_TYPE_FLOAT,
		SPIRV_TYPE_VECTOR,
		SPIRV_TYPE_MATRIX,
		SPIRV_TYPE_IMAGE,
		SPIRV_TYPE_SAMPLER,
		SPIRV_TYPE_SAMPLED_IMAGE,
		SPIRV_TYPE_ARRAY,
		SPIRV_TYPE_RUNTIME_ARRAY,
		SPIRV_TYPE_STRUCT,
		SPIRV_TYPE_OPAQUE,
		SPIRV_TYPE_POINTER,
		SPIRV_TYPE_FUNCTION,
		SPIRV_TYPE_EVENT,			// <VVV not implemented
		SPIRV_TYPE_DEVICE_EVENT,
		SPIRV_TYPE_RESERVE_ID,
		SPIRV_TYPE_QUEUE,
		SPIRV_TYPE_PIPE,
		SPIRV_TYPE_FORWARD_POINTER,
		SPIRV_TYPE_PIPE_STORAGE,
		SPIRV_TYPE_NAMED_BARRIER,
		SPIRV_TYPE_RAY_QUERY_PROVISIONAL,
		SPIRV_TYPE_ACCELERATION_STRUCTURE,
		SPIRV_TYPE_COOPERATIVE_MATRIX
	};

	enum SpirvImageDimension
	{
		SPIRV_IMAGE_1D,
		SPIRV_IMAGE_2D,
		SPIRV_IMAGE_3D,
		SPIRV_IMAGE_CUBE,
		SPIRV_IMAGE_RECT,
		SPIRV_IMAGE_BUFFER,
		SPIRV_IMAGE_SUBPASS_DATA
	};

	struct SpirvDecoration
	{
		uInt32	set;
		uInt32	location;
		uInt32	binding;
		bool	block;
		bool	bufferBlock;
	};

	struct SpirvIntType
	{
		uInt32 width;
		uInt32 signedness;
	};

	struct SpirvFloatType
	{
		uInt32 width;
	};

	struct SpirvVectorType
	{
		uInt32 typeId;
		uInt32 count;
	};

	struct SpirvMatrixType
	{
		uInt32 typeId;
		uInt32 columnCount;
	};

	struct SpirvImageType
	{
		uInt32				sampledTypeId;
		SpvDim				dimension;
		uInt32				depth;
		uInt32				arrayed;
		uInt32				multisampled;
		uInt32				sampled;
		SpvImageFormat		imageFormat;
		SpvAccessQualifier	access;
	};

	struct SpirvSampledImage
	{
		uInt32 imageTypeId;
	};

	struct SpirvArrayType
	{
		uInt32 typeId;
		uInt32 length;
	};

	struct SpirvRuntimeArrayType
	{
		uInt32 typeId;
	};

#define SPIRV_MAX_STRUCT_SIZE 16

	struct SprivStructType
	{
		uInt32 structEntryIds[SPIRV_MAX_STRUCT_SIZE];
	};

	struct SpirvOpaqueType
	{
		String name;
	};

	struct SprivPointerType
	{
		SpvStorageClass storageClass;
		uInt32 typeId;
	};

#define SPIRV_MAX_FUNCTION_PARAMETER_SIZE 15

	struct SpirvFunctionType
	{
		uInt32 returnTypeId;
		uInt32 paramaterTypeIds[15];
	};

	struct SpirvObject
	{
		String			name;
		SpirvType		type;
		uInt32			variableId;
		SpvStorageClass	storageClass;
		SpirvDecoration	decoration;
		bool			isMember;

		// To prevent default objects having the
		// SpvStorageClassUniformConstant (0) storage class
		SpirvObject() :
			name(),
			type(SPIRV_TYPE_UNKNOWN),
			variableId(-1),
			storageClass(SpvStorageClass(-1)),
			decoration{},
			isMember(false),
			structType{},
			opaqueType{}
		{}

		union
		{
			SpirvIntType			intType;
			SpirvFloatType			floatType;
			SpirvVectorType			vectorType;
			SpirvMatrixType			matrixType;
			SpirvImageType			imageType;
			SpirvSampledImage		sampledImageType;
			SpirvArrayType			arrayType;
			SpirvRuntimeArrayType	runtimeArrayType;
			SprivStructType			structType;
			SprivPointerType		pointerType;
			SpirvFunctionType		functionType;
		};

		// Since String can't be used in a union, SpirvOpaqueType 
		// must be separate from the union
		SpirvOpaqueType	opaqueType;
	};

	struct SpirvReflection
	{
		String				entryName;
		SpvExecutionModel	executionModel;
		VkShaderStageFlags	shaderStage;
		Array<SpirvObject>	objects;
	};

	struct SpirvUniform
	{
		String				name;
		uInt32				set;
		uInt32				binding;
		bool				isBlock;
		uInt32				sizeBytes;
		VkDescriptorType	descriptorType;
		VkShaderStageFlags	shaderStage;
	};

	struct SpirvAttribute
	{
		String		name;
		uInt32		location;
		uInt32		binding;
		VkFormat	format;
	};

	bool SpirvParseReflection(SpirvReflection* pReflection, const Array<uInt8>& spirvCode);

	void SpirvExtractUniforms(Array<SpirvUniform>& uniforms, SpirvReflection& reflection);

	void SpirvExtractAttributes(Array<SpirvAttribute>& attributes, SpirvReflection& reflection);

	uInt32 SpirvObjectSize(const SpirvObject& object, SpirvReflection& reflection);

	VkShaderStageFlagBits SpvExecutionModelToVkShaderStageFlags(SpvExecutionModel executionModel);
}