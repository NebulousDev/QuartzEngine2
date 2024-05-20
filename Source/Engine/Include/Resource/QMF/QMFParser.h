#pragma once

#include "QMF.h"
#include "EngineAPI.h"
#include "Types/Array.h"
#include "Memory/Allocator.h"

#include "Filesystem/Filesystem.h"
#include "Resource/Assets/Model.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API QMFParser
	{
	private:
		File&						mFile;
		QMF							mHeader;
		Model*						mpModel;
		Array<String>				mStrings;
		PoolAllocator<Model>*		mpModelAllocator;
		PoolAllocator<ByteBuffer>*	mpBufferAllocator;

	private:
		QMF			WriteBlankQMFHeader();
		QMFStringID RegisterString(const String& string);
		bool		BeginWriting();
		bool		EndWriting();
		bool		WriteStrings();
		bool		WriteMesh(const Mesh& mesh, const VertexData& vertexData);

		bool		BeginReading();
		bool		EndReading();
		bool		ReadStrings();
		bool		ReadMeshes();

	public:
		QMFParser(File& qmfFile,
			PoolAllocator<Model>* pModelAllocator = nullptr,
			PoolAllocator<ByteBuffer>* pBufferAllocator = nullptr);

		void SetModel(const Model& model);

		bool Read();
		bool Write();

		Model* GetModel() const { return mpModel; }
		const Array<String>& GetStrings() const { return mStrings; }
	};
}