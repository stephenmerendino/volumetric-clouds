#pragma once
#pragma warning(disable: 4201)

#include "Engine/Core/BinaryStream.hpp"

class UIntVector4
{
public:
	union{
		struct{
			unsigned int x, y, z, w;
		};

		unsigned int values[4];
	};

	UIntVector4();
	UIntVector4(const UIntVector4& copy);
	explicit UIntVector4(unsigned int initialX, unsigned int initialY, unsigned int initialZ, unsigned int initialW);

	bool					operator==(const UIntVector4& vectorToEqual)															const;
	bool					operator!=(const UIntVector4& vectorToNotEqual)															const;
	UIntVector4				operator+(const UIntVector4& vectorToAdd)																const;
	UIntVector4				operator-(const UIntVector4& vectorToSubtract)															const;
	UIntVector4				operator*(const UIntVector4& perAxisScaleFactors)														const;
	void					operator*=(const UIntVector4& perAxisScaleFactors);
	void					operator+=(const UIntVector4& vectorToAdd);
	void					operator-=(const UIntVector4& vectorToSubtract);

	static const UIntVector4	ZERO;
};

template<>
inline
bool BinaryStream::write(const UIntVector4& v)
{
	return write(v.x) && write(v.y) && write(v.z) && write(v.w);
}

template<>
inline
bool BinaryStream::read(UIntVector4& v)
{
	return read(v.x) && read(v.y) && read(v.z) && read(v.w);
}