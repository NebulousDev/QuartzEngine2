#pragma once

#include "QShader.h"
#include "EngineAPI.h"
#include "Types/Array.h"
#include "Memory/Allocator.h"

#include "Filesystem/Filesystem.h"
#include "Resource/Assets/Shader.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API QShaderParser
	{
	private:
		File&						mFile;
		QShader						mHeader;
		Shader*						mpShader;
		Array<String>				mStrings;
		PoolAllocator<Shader>*		mpShaderAllocator;
		PoolAllocator<ByteBuffer>*	mpBufferAllocator;

	private:
		QShader		WriteBlankQShaderHeader();
		QStringID	RegisterString(const String& string);
		bool		BeginWriting();
		bool		EndWriting();
		bool		WriteStrings();
		bool		WriteShaderParam(const ShaderParam& shaderParam);
		bool		WriteShaderCode(const ShaderCode& shaderCode);

		bool		BeginReading();
		bool		EndReading();
		bool		ReadStrings();
		bool		ReadShaderParams();
		bool		ReadShaderCodes();

	public:
		QShaderParser(File& qShaderFile,
			PoolAllocator<Shader>* pShaderAllocator = nullptr,
			PoolAllocator<ByteBuffer>* pBufferAllocator = nullptr);

		void SetShader(const Shader& shader);

		bool Read();
		bool Write();

		Shader* GetShader() const { return mpShader; }
		const Array<String>& GetStrings() const { return mStrings; }
	};
}