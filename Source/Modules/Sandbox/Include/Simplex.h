#pragma once

#include "Math/Vector.h"
#include "PhysicsTypes.h"

namespace Quartz
{
	class Simplex
	{
	private:
		Vec3p mPoints[4];
		uSize mSize;

	private:
		bool Line(Vec3p& inOutDir);
		bool Triangle(Vec3p& inOutDir);
		bool Tetrahedron(Vec3p& inOutDir);

	public:
		Simplex();

		void Push(const Vec3p& point);
		uSize Size() const;

		bool Next(Vec3p& inOutDir);

		const Vec3p& operator[](uSize index) const;
	};
}