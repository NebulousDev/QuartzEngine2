#pragma once

#include "Types/Types.h"

namespace Quartz
{
	template<typename _HandleIntType, typename _VersionIntType, uSize versionBits>
	class _Entity
	{
	public:
		using HandleIntType    = _HandleIntType;
		using VersionIntType   = _VersionIntType;
		using SparseHandleType = _Entity<_HandleIntType, _VersionIntType, versionBits>;

	private:
		constexpr static uSize INDEX_BITS   = (sizeof(HandleIntType) * 8) - versionBits;
		constexpr static uSize VERSION_BITS = versionBits;
		constexpr static uSize NULL_HANDLE  = HandleIntType(0);

	public:
		union
		{
			struct
			{
				/* The index into the sparse array */
				HandleIntType index : INDEX_BITS;

				/* The version of the handle */
				VersionIntType version : VERSION_BITS;
			};

			/* The compact sparse handle value */
			HandleIntType handle;
		};

		operator HandleIntType() const
		{
			return handle;
		}

		constexpr _Entity()
			: handle(NULL_HANDLE) {}

		constexpr _Entity(HandleIntType handle)
			: handle(handle) {}

		constexpr _Entity(HandleIntType index, VersionIntType version)
			: index(index), version(version) {}
	};

	/* 4-bits version, 12-bits index */
	using Entity16 = _Entity<uInt16, uInt8, 4>;

	/* 8-bits version, 24-bits index */
	using Entity32 = _Entity<uInt32, uInt8, 8>;

	/* 16-bits version, 48-bits index */
	using Entity64 = _Entity<uInt64, uInt16, 16>;

	/* Standard Entity */
	using Entity = Entity32;

	/* Null Entity */
	constexpr Entity NullEntity(0);
}