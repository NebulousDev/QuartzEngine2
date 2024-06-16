#pragma once

#include "Resource/Binary/QShaderParser.h"

#include "Log.h"

namespace Quartz
{
	QShader QShaderParser::WriteBlankQShaderHeader()
	{
		QStringTable stringTable;
		stringTable.encoding		= STRING_ENCODING_UTF8;
		stringTable._reserved0		= 0;
		stringTable.stringCount		= 0;
		stringTable.strsOffset		= 0;
		stringTable.strsSizeBytes	= 0;
		stringTable.nextExtOffset	= 0;

		QShaderParamTable paramTable;
		paramTable.paramsCount		= 0;
		paramTable._reserved0		= 0;
		paramTable.paramsOffset		= 0;
		paramTable.paramsSizeBytes	= 0;
		paramTable.nextExtOffset	= 0;

		QShaderShaderTable shaderTable;
		shaderTable.shaderCount			= 0;
		shaderTable._reserved0			= 0;
		shaderTable.shadersOffset		= 0;
		shaderTable.shadersSizeBytes	= 0;
		shaderTable.nextExtOffset		= 0;

		QShader qShader;			// Magic assigned in header
		qShader.versionMajor		= 1;
		qShader.versionMinor		= 0;
		qShader._reserved0			= 0;
		qShader.stage				= QSHADER_STAGE_INVALID;
		qShader.sourceLangs			= 0;
		qShader.stringTable			= stringTable;
		qShader.paramTable			= paramTable;
		qShader.shaderTable			= shaderTable;
		qShader.nextExtOffset		= 0;

		mFile.WriteValues<QShader>(&qShader, 1);

		return qShader;
	}

	QStringID QShaderParser::RegisterString(const String& string)
	{
		mStrings.PushBack(string);
		return mStrings.Size() - 1;
	}

	bool QShaderParser::BeginWriting()
	{
		if (mFile.IsOpen())
		{
			LogWarning("QShader File [%s] is already open.", mFile.GetPath().Str());
			mFile.SetFilePtr(0, FILE_PTR_BEGIN);
		}
		else if (!mFile.Open(FILE_OPEN_WRITE | FILE_OPEN_BINARY | FILE_OPEN_CLEAR | FILE_OPEN_SHARE_READ))
		{
			LogError("QShader File [%s] failed to open.", mFile.GetPath().Str());
			return false;
		}

		mHeader = WriteBlankQShaderHeader();

		mHeader.stage = (QShaderStage)mpShader->stage;

		return true;
	}

	bool QShaderParser::EndWriting()
	{
		mFile.SetFilePtr(0, FILE_PTR_BEGIN);

		if (!mFile.WriteValues<QShader>(&mHeader, 1))
		{
			return false;
		}

		mFile.Close();

		return true;
	}

	bool QShaderParser::WriteStrings()
	{
		mHeader.stringTable.strsOffset = mFile.GetFilePtr();

		for (const String& string : mStrings)
		{
			uInt16 strLength = string.Length();

			if (!mFile.WriteValues<uInt16>(&strLength, 1))
			{
				return false;
			}

			if (!mFile.Write((uInt8*)string.Str(), string.Length() + 1))
			{
				return false;
			}

			mHeader.stringTable.stringCount++;
			mHeader.stringTable.strsSizeBytes += 2 + strLength + 1;
		}

		return true;
	}

	bool QShaderParser::WriteShaderParam(const ShaderUniform& shaderParam)
	{
		QShaderParamTable& paramTable = mHeader.paramTable;

		uInt64 fileOffset = mFile.GetFilePtr();

		if (paramTable.paramsOffset == 0)
		{
			paramTable.paramsOffset = fileOffset;
		}

		QShaderParam qShaderParam;
		qShaderParam.nameID				= RegisterString(shaderParam.name);
		qShaderParam._reserved0			= 0;
		qShaderParam.type				= (QShaderParamType)shaderParam.type;
		qShaderParam.set				= shaderParam.set;
		qShaderParam.binding			= shaderParam.binding;
		qShaderParam.arrayCount			= shaderParam.arrayCount;
		qShaderParam._reserved1			= 0;
		qShaderParam.valueOffsetBytes	= shaderParam.offsetBytes;
		qShaderParam.valueSizeBytes		= shaderParam.sizeBytes;

		if (!mFile.WriteValues<QShaderParam>(&qShaderParam, 1))
		{
			return false;
		}

		paramTable.paramsSizeBytes += sizeof(QShaderParam);
		paramTable.paramsCount++;

		return true;
	}

	bool QShaderParser::WriteShaderCode(const Shader::ShaderCode& shaderCode)
	{
		QShaderShaderTable& shaderTable = mHeader.shaderTable;

		uInt64 fileOffset = mFile.GetFilePtr();

		if (shaderTable.shadersOffset == 0)
		{
			shaderTable.shadersOffset = fileOffset;
		}

		QShaderCode qShaderCode;
		qShaderCode.entryID = RegisterString(shaderCode.entry);

		switch (shaderCode.lang)
		{
			case SHADER_LANG_GLSL_TEXT:		qShaderCode.lang = QSHADER_LANG_GLSL_TEXT;	break;
			case SHADER_LANG_GLSL_SPIRV:	qShaderCode.lang = QSHADER_LANG_GLSL_SPIRV;	break;
			case SHADER_LANG_HLSL_TEXT:		qShaderCode.lang = QSHADER_LANG_HLSL_TEXT;	break;
			case SHADER_LANG_HLSL_DXBC:		qShaderCode.lang = QSHADER_LANG_HLSL_DXBC;	break;
			case SHADER_LANG_HLSL_DXIL:		qShaderCode.lang = QSHADER_LANG_HLSL_DXIL;	break;
			default: break;
		}

		mHeader.sourceLangs |= qShaderCode.lang;

		qShaderCode.codeOffset		= fileOffset + sizeof(QShaderCode);
		qShaderCode.codeSizeBytes	= shaderCode.pSourceBuffer->Size();

		if (!mFile.WriteValues<QShaderCode>(&qShaderCode, 1))
		{
			return false;
		}

		shaderTable.shadersSizeBytes += sizeof(QShaderCode);

		if (!mFile.Write(shaderCode.pSourceBuffer->Data(), qShaderCode.codeSizeBytes))
		{
			return false;
		}

		shaderTable.shadersSizeBytes += qShaderCode.codeSizeBytes;
		shaderTable.shaderCount++;

		return true;
	}

	bool QShaderParser::BeginReading()
	{
		if (mFile.IsOpen())
		{
			LogWarning("QShader File [%s] is already open.", mFile.GetPath().Str());
			mFile.SetFilePtr(0, FILE_PTR_BEGIN);
		}
		else if (!mFile.Open(FILE_OPEN_READ | FILE_OPEN_BINARY | FILE_OPEN_SHARE_READ))
		{
			LogError("Error reading QShader File [%s]. Failed to open.", mFile.GetPath().Str());
			return false;
		}

		if (!mFile.ReadValues<QShader>(&mHeader, 1))
		{
			return false;
		}

		if (WrapperString(mHeader.magic, 7) != "QShader"_STR)
		{
			LogError("Error reading QShader File [%s]. File is not a valid QShader file.", mFile.GetPath().Str());
			return false;
		}

		mpShader = mpShaderAllocator->Allocate(&mFile);

		if (!mpShader)
		{
			// @TODO: error
			return false;
		}

		mpShader->name	= mFile.GetPath();
		mpShader->stage = (ShaderStage)mHeader.stage;

		return true;
	}

	bool QShaderParser::EndReading()
	{
		mFile.Close();

		return true;
	}

	bool QShaderParser::ReadStrings()
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

			if (!mFile.Read((uInt8*)string.Data(), strLength + 1))
			{
				return false;
			}

			mStrings.PushBack(string);
		}

		return true;
	}

	bool QShaderParser::ReadShaderParams()
	{
		if (mHeader.paramTable.paramsCount == 0)
		{
			return true;
		}

		mFile.SetFilePtr(mHeader.paramTable.paramsOffset, FILE_PTR_BEGIN);

		for (uSize i = 0; i < mHeader.paramTable.paramsCount; i++)
		{
			QShaderParam qShaderParam;

			if (!mFile.ReadValues<QShaderParam>(&qShaderParam, 1))
			{
				return false;
			}

			ShaderUniform shaderParam;

			if (qShaderParam.nameID < mStrings.Size())
			{
				shaderParam.name = mStrings[qShaderParam.nameID];
			}
			else
			{
				return false;
			}

			shaderParam.type			= (ShaderUniformType)qShaderParam.type;
			shaderParam.set				= qShaderParam.set;
			shaderParam.binding			= qShaderParam.binding;
			shaderParam.arrayCount		= qShaderParam.arrayCount;
			shaderParam.offsetBytes		= qShaderParam.valueOffsetBytes;
			shaderParam.sizeBytes		= qShaderParam.valueSizeBytes;
			
			mpShader->uniforms.PushBack(shaderParam);
		}

		return true;
	}

	bool QShaderParser::ReadShaderCodes()
	{
		if (mHeader.shaderTable.shaderCount == 0)
		{
			return true;
		}

		mFile.SetFilePtr(mHeader.shaderTable.shadersOffset, FILE_PTR_BEGIN);

		for (uSize i = 0; i < mHeader.shaderTable.shaderCount; i++)
		{
			QShaderCode qShaderCode;

			if (!mFile.ReadValues<QShaderCode>(&qShaderCode, 1))
			{
				return false;
			}

			Shader::ShaderCode shaderCode;

			if (qShaderCode.entryID < mStrings.Size())
			{
				shaderCode.entry = mStrings[qShaderCode.entryID];
			}
			else
			{
				return false;
			}

			switch (qShaderCode.lang)
			{
				case QSHADER_LANG_GLSL_TEXT:	shaderCode.lang = SHADER_LANG_GLSL_TEXT;	break;
				case QSHADER_LANG_GLSL_SPIRV:	shaderCode.lang = SHADER_LANG_GLSL_SPIRV;	break;
				case QSHADER_LANG_HLSL_TEXT:	shaderCode.lang = SHADER_LANG_HLSL_TEXT;	break;
				case QSHADER_LANG_HLSL_DXBC:	shaderCode.lang = SHADER_LANG_HLSL_DXBC;	break;
				case QSHADER_LANG_HLSL_DXIL:	shaderCode.lang = SHADER_LANG_HLSL_DXIL;	break;
				default: shaderCode.lang = SHADER_LANG_INVALID;
			}

			shaderCode.pSourceBuffer = mpBufferAllocator->Allocate(qShaderCode.codeSizeBytes);
			shaderCode.pSourceBuffer->Allocate(qShaderCode.codeSizeBytes);

			mFile.SetFilePtr(qShaderCode.codeOffset, FILE_PTR_BEGIN);

			if (!mFile.Read(shaderCode.pSourceBuffer->Data(), qShaderCode.codeSizeBytes))
			{
				mpBufferAllocator->Free(shaderCode.pSourceBuffer);
				return false;
			}

			mpShader->shaderCodes.PushBack(shaderCode);
		}

		return true;
	}

	QShaderParser::QShaderParser(File& qShaderFile, 
		PoolAllocator<Shader>* pShaderAllocator,
		PoolAllocator<ByteBuffer>* pBufferAllocator) :
		mFile(qShaderFile),
		mHeader{},
		mpShaderAllocator(pShaderAllocator),
		mpBufferAllocator(pBufferAllocator) { }

	void QShaderParser::SetShader(const Shader& shader)
	{
		mpShader = const_cast<Shader*>(&shader);
	}

	bool QShaderParser::Read()
	{
		if (!BeginReading())
		{
			mFile.Close();
			return false;
		}

		if (!ReadStrings())
		{
			mpShaderAllocator->Free(mpShader);
			mFile.Close();
			return false;
		}

		if (!ReadShaderParams())
		{
			mpShaderAllocator->Free(mpShader);
			mFile.Close();
			return false;
		}

		if (!ReadShaderCodes())
		{
			for (Shader::ShaderCode& code : mpShader->shaderCodes)
			{
				mpBufferAllocator->Free(code.pSourceBuffer);
			}

			mpShaderAllocator->Free(mpShader);
			mFile.Close();
			return false;
		}

		if (!EndReading())
		{
			for (Shader::ShaderCode& code : mpShader->shaderCodes)
			{
				mpBufferAllocator->Free(code.pSourceBuffer);
			}

			mpShaderAllocator->Free(mpShader);
			mFile.Close();
			return false;
		}

		return true;
	}

	bool QShaderParser::Write()
	{
		if (!mpShader)
		{
			return false;
		}

		if (!BeginWriting())
		{
			mFile.Close();
			return false;
		}

		for (const ShaderUniform& param : mpShader->uniforms)
		{
			if (!WriteShaderParam(param))
			{
				mFile.Close();
				return false;
			}
		}

		for (const Shader::ShaderCode& code : mpShader->shaderCodes)
		{
			if (!WriteShaderCode(code))
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