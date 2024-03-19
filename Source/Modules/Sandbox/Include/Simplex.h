#pragma once

#include "Math/Vector.h"

namespace Quartz
{
	class Simplex
	{
	private:
		Vec3f mPoints[4];
		uSize mSize;

	private:
		bool Line(Vec3f& inOutDir);
		bool Triangle(Vec3f& inOutDir);
		bool Tetrahedron(Vec3f& inOutDir);

	public:
		Simplex();

		void Push(const Vec3f& point);
		uSize Size() const;

		bool Next(Vec3f& inOutDir);

		const Vec3f& operator[](uSize index) const;
	};
}