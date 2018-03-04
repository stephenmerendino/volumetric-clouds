#pragma once
#pragma warning(disable: 4201)

#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/BinaryStream.hpp"

class Vector4{
public:

	union{
		struct{
			float x, y, z, w;
		};

		struct{
			Vector3 xyz;
			float w;
		};

		float values[4];
	};

	Vector4();
	Vector4(const Vector4& copy);
	Vector4(const Vector3& vec3, float w);
	explicit Vector4(float initialX, float initialY, float initialZ, float initialW);

	void					GetXYZW(float &outX, float &outY, float &outZ, float &outW)												const;
	const float*			GetAsFloatArray()																						const;
	float*					GetAsFloatArray();
	float					CalcLength3D()																							const;
	float					CalcLengthSquared3D()																					const;
	float					CalcLength4D()																							const;
	float					CalcLengthSquared4D()																					const;
	bool					IsMostlyEqual(const Vector4& vectorToEqual, float epsilon = 0.001f)										const;
	bool					IsMostlyEqual(float compareX, float compareY, float compareZ, float compareW, float epsilon = 0.001f)	const;

	void					SetXYZW(float newX, float newY, float newZ, float newW);
	float					Normalize3D();
	Vector4					Normalized3D()																							const;
	float					Normalize4D();
	Vector4					Normalized4D()																							const;
	float					SetLength3D(float newLength);
	float					SetLength4D(float newLength);
	void					ScaleUniform3D(float scale);
	void					ScaleUniform4D(float scale);
	void					ScaleNonUniform(const Vector4& perAxisScaleFactors);
	void					ScaleNonUniform(float scaleX, float scaleY, float scaleZ, float scaleW);
	void					InverseScaleNonUniform(const Vector4& perAxisDivisors);
	void					InverseScaleNonUniform(float inverseScaleX, float inverseScaleY, float inverseScaleZ, float inverseScaleW);

	bool					operator==(const Vector4& vectorToEqual)																const;
	bool					operator!=(const Vector4& vectorToNotEqual)																const;
	Vector4					operator+(const Vector4& vectorToAdd)																	const;
	Vector4					operator-(const Vector4& vectorToSubtract)																const;
	Vector4					operator*(float scale)																					const;
	Vector4					operator*(const Vector4& perAxisScaleFactors)															const;
	Vector4					operator/(float inverseScale)																			const;
	Vector4					operator-();
	void					operator*=(float scale);
	void					operator*=(const Vector4& perAxisScaleFactors);
	void					operator+=(const Vector4& vectorToAdd);
	void					operator-=(const Vector4& vectorToSubtract);
	void					operator/=(float inverseScale);

	friend float			CalcDistance(const Vector4& start, const Vector4& end);
	friend float			CalcDistanceSquared(const Vector4& start, const Vector4& end);
	friend const Vector4	operator*(float scale, const Vector4& vectorToScale);
	friend float			DotProduct(const Vector4& a, const Vector4& b);
	friend bool				AreMostlyEqual(const Vector4& a, const Vector4& b, float epsilon = 0.001f);
	friend Vector4			Interpolate(const Vector4& start, const Vector4& end, float fractionToEnd);

	static const Vector4	ZERO;
};

template<>
inline
bool BinaryStream::write(const Vector4& v)
{
	return write(v.x) && write(v.y) && write(v.z) && write(v.w);
}

template<>
inline
bool BinaryStream::read(Vector4& v)
{
	return read(v.x) && read(v.y) && read(v.z) && read(v.w);
}