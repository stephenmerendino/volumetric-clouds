#pragma once
#pragma warning(disable: 4201)

class IntVector4
{
public:
	union{
		struct{
			int x, y, z, w;
		};

		int values[4];
	};

	IntVector4();
	IntVector4(const IntVector4& copy);
	explicit IntVector4(int initialX, int initialY, int initialZ, int initialW);

	bool					operator==(const IntVector4& vectorToEqual)																const;
	bool					operator!=(const IntVector4& vectorToNotEqual)															const;
	IntVector4				operator+(const IntVector4& vectorToAdd)																const;
	IntVector4				operator-(const IntVector4& vectorToSubtract)															const;
	IntVector4				operator*(const IntVector4& perAxisScaleFactors)														const;
	IntVector4				operator-();
	void					operator*=(const IntVector4& perAxisScaleFactors);
	void					operator+=(const IntVector4& vectorToAdd);
	void					operator-=(const IntVector4& vectorToSubtract);

	static const IntVector4	ZERO;
};