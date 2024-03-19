#include "Simplex.h"

namespace Quartz
{
	Simplex::Simplex() : mSize(0), mPoints{} {}

	void Simplex::Push(const Vec3f& point)
	{
		// @TODO: assert mSize < 4
		mPoints[mSize++] = point;
	}

	uSize Simplex::Size() const
	{
		return mSize;
	}

	bool Simplex::Line(Vec3f& inOutDir)
	{
		Vec3f a = mPoints[1];
		Vec3f b = mPoints[0];

		Vec3f ab = b - a;
		Vec3f ao = -a;

		if (Dot(ab, ao) > 0.0f)
		{
			inOutDir = Cross(Cross(ab, ao), ab);
		}

		return false;
	}

	bool Simplex::Triangle(Vec3f& inOutDir)
	{
		Vec3f a = mPoints[2];
		Vec3f b = mPoints[1];
		Vec3f c = mPoints[0];

		Vec3f ab = b - a;
		Vec3f ac = c - a;
		Vec3f ao = -a;
 
		Vec3f abc = Cross(ab, ac);

		if (Dot(Cross(abc, ac), ao) > 0.0f)
		{
			if (Dot(ac, ao) > 0.0f)
			{
				mPoints[0] = a;
				mPoints[1] = c;
				mSize = 2;

				inOutDir = Cross(Cross(ac, ao), ac);
			}

			else
			{
				if (Dot(ab, ao) > 0.0f)
				{
					mPoints[0] = a;
					mPoints[1] = b;
					mSize = 2;

					inOutDir = Cross(Cross(ab, ao), ab);
				}
				else
				{
					mPoints[0] = a;
					mSize = 1;

					inOutDir = ao;
				}
			}
		}
 
		else if (Dot(Cross(ab, abc), ao) > 0.0f)
		{
			if (Dot(ab, ao) > 0.0f)
			{
				mPoints[0] = a;
				mPoints[1] = b;
				mSize = 2;

				inOutDir = Cross(Cross(ab, ao), ab);
			}
			else
			{
				mPoints[0] = a;
				mSize = 1;

				inOutDir = ao;
			}
		}

		else // Inside the triangle
		{
			if (Dot(abc, ao) > 0.0f)
			{
				mPoints[0] = b;
				mPoints[1] = c;
				mPoints[2] = a;
				mSize = 3;

				inOutDir = abc;
			}

			else // Opposite winding
			{
				inOutDir = -abc;
			}
		}

		return false;
	}

	bool Simplex::Tetrahedron(Vec3f& inOutDir)
	{
		Vec3f a = mPoints[3];
		Vec3f b = mPoints[2];
		Vec3f c = mPoints[1];
		Vec3f d = mPoints[0];

		Vec3f ab = b - a;
		Vec3f ac = c - a;
		Vec3f ad = d - a;
		Vec3f ao = -a;
 
		Vec3f acb = Cross(ac, ab);
		Vec3f adc = Cross(ad, ac);
		Vec3f abd = Cross(ab, ad);
 
		if (Dot(acb, ao) > 0.0f)
		{
			mPoints[0] = a;
			mPoints[1] = c;
			mPoints[2] = b;
			mSize = 3;

			return Triangle(inOutDir);
		}
		
		else if (Dot(adc, ao) > 0.0f)
		{
			mPoints[0] = a;
			mPoints[1] = d;
			mPoints[2] = c;
			mSize = 3;

			return Triangle(inOutDir);
		}
 
		else if (Dot(abd, ao) > 0.0f)
		{
			mPoints[0] = a;
			mPoints[1] = b;
			mPoints[2] = d;
			mSize = 3;

			return Triangle(inOutDir);
		}
 
		return true;
	}

	bool Simplex::Next(Vec3f& inOutDir)
	{
		switch (mSize)
		{
			case 2: return Line(inOutDir);
			case 3: return Triangle(inOutDir);
			case 4: return Tetrahedron(inOutDir);
			default: return false;
		}
	}

	const Vec3f& Simplex::operator[](uSize index) const
	{
		return mPoints[index];
	}
}