#ifndef __NOISE_H__
#define __NOISE_H__

// Easing function used by Ken Perlin's original algorithm
float SmoothStep3(float t)
{
	return t * t * (3 - 2 * t);
}

// Easing function used by Ken Perlin's "Improved Perlin's Noise (2002)"
// 6t^5 - 15t^4 + 10t^3
float SmoothStep5(float t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

//-----------------------------------------------------------------------------------------------
// Returns an unsigned integer containing 32 reasonably-well-scrambled bits, based on a given
//	(signed) integer input parameter (position/index) and [optional] seed.  Kind of like looking
//	up a value in an infinitely large [non-existent] table of previously generated random numbers.
//
// The base bit-noise constants were designed to have distinctive and interesting bits,
//	and have so far produced seemingly excellent experimental test results.
//
unsigned int Get1dNoiseUint( int positionX, unsigned int seed )
{
	const unsigned int BIT_NOISE1 = 0x68E31DA4; // 0b0110'1000'1110'0011'0001'1101'1010'0100;
	const unsigned int BIT_NOISE2 = 0xB5297A4D; // 0b1011'0101'0010'1001'0111'1010'0100'1101;
	const unsigned int BIT_NOISE3 = 0x1B56C4E9; // 0b0001'1011'0101'0110'1100'0100'1110'1001;

	unsigned int mangledBits = (unsigned int) positionX;
	mangledBits *= BIT_NOISE1;
	mangledBits += seed;
	mangledBits ^= (mangledBits >> 8);
	mangledBits += BIT_NOISE2;
	mangledBits ^= (mangledBits << 8);
	mangledBits *= BIT_NOISE3;
	mangledBits ^= (mangledBits >> 8);
	return mangledBits;
}

inline unsigned int Get2dNoiseUint(int indexX, int indexY, unsigned int seed)
{
	const int PRIME_NUMBER = 198491317; // Large prime number with non-boring bits
	return Get1dNoiseUint(indexX + (PRIME_NUMBER * indexY), seed);
}

inline unsigned int Get3dNoiseUint(int indexX, int indexY, int indexZ, unsigned int seed)
{
	const int PRIME1 = 198491317; // Large prime number with non-boring bits
	const int PRIME2 = 6542989; // Large prime number with distinct and non-boring bits
	return Get1dNoiseUint(indexX + (PRIME1 * indexY) + (PRIME2 * indexZ), seed);
}

inline unsigned int Get4dNoiseUint(int indexX, int indexY, int indexZ, int indexT, unsigned int seed)
{
	const int PRIME1 = 198491317; // Large prime number with non-boring bits
	const int PRIME2 = 6542989; // Large prime number with distinct and non-boring bits
	const int PRIME3 = 357239; // Large prime number with distinct and non-boring bits
	return Get1dNoiseUint(indexX + (PRIME1 * indexY) + (PRIME2 * indexZ) + (PRIME3 * indexT), seed);
}

inline float Get1dNoiseZeroToOne(int index, unsigned int seed)
{
	const double ONE_OVER_MAX_UINT = (1.0 / (double)0xFFFFFFFF);
	return (float)(ONE_OVER_MAX_UINT * (double)Get1dNoiseUint(index, seed));
}

inline float Get2dNoiseZeroToOne(int indexX, int indexY, unsigned int seed)
{
	const double ONE_OVER_MAX_UINT = (1.0 / (double)0xFFFFFFFF);
	return (float)(ONE_OVER_MAX_UINT * (double)Get2dNoiseUint(indexX, indexY, seed));
}

inline float Get3dNoiseZeroToOne(int indexX, int indexY, int indexZ, unsigned int seed)
{
	const double ONE_OVER_MAX_UINT = (1.0 / (double)0xFFFFFFFF);
	return (float)(ONE_OVER_MAX_UINT * (double)Get3dNoiseUint(indexX, indexY, indexZ, seed));
}

inline float Get4dNoiseZeroToOne(int indexX, int indexY, int indexZ, int indexT, unsigned int seed)
{
	const double ONE_OVER_MAX_UINT = (1.0 / (double)0xFFFFFFFF);
	return (float)(ONE_OVER_MAX_UINT * (double)Get4dNoiseUint(indexX, indexY, indexZ, indexT, seed));
}

inline float Get1dNoiseNegOneToOne(int index, unsigned int seed)
{
	const double ONE_OVER_MAX_INT = (1.0 / (double)0x7FFFFFFF);
	return (float)(ONE_OVER_MAX_INT * (double)(int)Get1dNoiseUint(index, seed));
}

inline float Get2dNoiseNegOneToOne(int indexX, int indexY, unsigned int seed)
{
	const double ONE_OVER_MAX_INT = (1.0 / (double)0x7FFFFFFF);
	return (float)(ONE_OVER_MAX_INT * (double)(int)Get2dNoiseUint(indexX, indexY, seed));
}

inline float Get3dNoiseNegOneToOne(int indexX, int indexY, int indexZ, unsigned int seed)
{
	const double ONE_OVER_MAX_INT = (1.0 / (double)0x7FFFFFFF);
	return (float)(ONE_OVER_MAX_INT * (double)(int)Get3dNoiseUint(indexX, indexY, indexZ, seed));
}

inline float Get4dNoiseNegOneToOne(int indexX, int indexY, int indexZ, int indexT, unsigned int seed)
{
	const double ONE_OVER_MAX_INT = (1.0 / (double)0x7FFFFFFF);
	return (float)(ONE_OVER_MAX_INT * (double)(int)Get4dNoiseUint(indexX, indexY, indexZ, indexT, seed));
}

//-----------------------------------------------------------------------------------------------
// Perlin noise is fractal noise with "gradient vector smoothing" applied.
//
// In 1D, the gradients are trivial: -1.0 or 1.0, so resulting noise is boring at one octave.

float Compute1dPerlinNoiseNegOneToOne(float position, unsigned int seed)
{
	const float2 gradients = { -1.f, 1.f }; // 1D unit "gradient" vectors; one back, one forward

	// Determine random "gradient vectors" (just +1 or -1 for 1D Perlin) for surrounding corners
	float positionFloor = (float)floor(position);
	int indexWest = (int)positionFloor;
	int indexEast = indexWest + 1;
	float gradientWest = gradients[Get1dNoiseUint(indexWest, seed) & 0x00000001];
	float gradientEast = gradients[Get1dNoiseUint(indexEast, seed) & 0x00000001];

	// Dot each point's gradient with displacement from point to position
	float displacementFromWest = position - positionFloor; // always positive
	float displacementFromEast = displacementFromWest - 1.f; // always negative
	float dotWest = gradientWest * displacementFromWest; // 1D "dot product" is... multiply
	float dotEast = gradientEast * displacementFromEast;

	// Do a smoothed (nonlinear) weighted average of dot results
	float weightEast = SmoothStep3(displacementFromWest);
	float weightWest = 1.f - weightEast;
	float blendTotal = (weightWest * dotWest) + (weightEast * dotEast);
	float noiseNegOneToOne = 2.f * blendTotal; // 1D Perlin is in [-.5,.5]; map to [-1,1]

	return noiseNegOneToOne;
}

float Compute1dPerlinNoiseNegOneToOne(float position, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed)
{
	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave

	float totalNoise = 0.f;
	float totalAmplitude = 0.f;
	float currentAmplitude = 1.f;
	float currentPosition = position * (1.f / scale);

	for (unsigned int octaveNum = 0; octaveNum < numOctaves; ++octaveNum)
	{
		float noiseThisOctave = Compute1dPerlinNoiseNegOneToOne(currentPosition, seed);

		// Accumulate results and prepare for next octave (if any)
		totalNoise += noiseThisOctave * currentAmplitude;
		totalAmplitude += currentAmplitude;
		currentAmplitude *= octavePersistence;
		currentPosition *= octaveScale;
		currentPosition += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		++seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if (renormalize && totalAmplitude > 0.f)
	{
		totalNoise /= totalAmplitude;				// Amplitude exceeds 1.0 if octaves are used
		totalNoise = (totalNoise * 0.5f) + 0.5f;	// Map to [0,1]
		totalNoise = SmoothStep3(totalNoise);		// Push towards extents (octaves pull us away)
		totalNoise = (totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
	}

	return totalNoise;
}

float Compute1dPerlinNoiseZeroToOne(float position, unsigned int seed)
{
	float noise = Compute1dPerlinNoiseNegOneToOne(position, seed);
	noise *= 0.5;
	noise += 0.5;
	return noise;
}

float Compute1dPerlinNoiseZeroToOne(float position, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed)
{
	float noise = Compute1dPerlinNoiseNegOneToOne(position, scale, numOctaves, octavePersistence, octaveScale, renormalize, seed);
	noise *= 0.5;
	noise += 0.5;
	return noise;
}

float Compute2dPerlinNoiseNegOneToOne(float x, float y, unsigned int seed)
{
	const float2 gradients[8] = // Normalized unit vectors in 8 quarter-cardinal directions
	{
		float2(+0.923879533f, +0.382683432f), //  22.5 degrees (ENE)
		float2(+0.382683432f, +0.923879533f), //  67.5 degrees (NNE)
		float2(-0.382683432f, +0.923879533f), // 112.5 degrees (NNW)
		float2(-0.923879533f, +0.382683432f), // 157.5 degrees (WNW)
		float2(-0.923879533f, -0.382683432f), // 202.5 degrees (WSW)
		float2(-0.382683432f, -0.923879533f), // 247.5 degrees (SSW)
		float2(+0.382683432f, -0.923879533f), // 292.5 degrees (SSE)
		float2(+0.923879533f, -0.382683432f)	 // 337.5 degrees (ESE)
	};

	// Determine random unit "gradient vectors" for surrounding corners
	float2 cellMins = float2(floor(x), floor(y));
	float2 cellMaxs = float2(cellMins.x + 1.f, cellMins.y + 1.f);
	int indexWestX = (int)cellMins.x;
	int indexSouthY = (int)cellMins.y;
	int indexEastX = indexWestX + 1;
	int indexNorthY = indexSouthY + 1;

	unsigned int noiseSW = Get2dNoiseUint(indexWestX, indexSouthY, seed);
	unsigned int noiseSE = Get2dNoiseUint(indexEastX, indexSouthY, seed);
	unsigned int noiseNW = Get2dNoiseUint(indexWestX, indexNorthY, seed);
	unsigned int noiseNE = Get2dNoiseUint(indexEastX, indexNorthY, seed);

	const float2 gradientSW = gradients[noiseSW & 0x00000007];
	const float2 gradientSE = gradients[noiseSE & 0x00000007];
	const float2 gradientNW = gradients[noiseNW & 0x00000007];
	const float2 gradientNE = gradients[noiseNE & 0x00000007];

	// Dot each corner's gradient with displacement from corner to position
	float2 displacementFromSW = float2(x - cellMins.x, y - cellMins.y);
	float2 displacementFromSE = float2(x - cellMaxs.x, y - cellMins.y);
	float2 displacementFromNW = float2(x - cellMins.x, y - cellMaxs.y);
	float2 displacementFromNE = float2(x - cellMaxs.x, y - cellMaxs.y);

	float dotSouthWest = dot(gradientSW, displacementFromSW);
	float dotSouthEast = dot(gradientSE, displacementFromSE);
	float dotNorthWest = dot(gradientNW, displacementFromNW);
	float dotNorthEast = dot(gradientNE, displacementFromNE);

	// Do a smoothed (nonlinear) weighted average of dot results
	float weightEast = SmoothStep3(displacementFromSW.x);
	float weightNorth = SmoothStep3(displacementFromSW.y);
	float weightWest = 1.f - weightEast;
	float weightSouth = 1.f - weightNorth;

	float blendSouth = (weightEast * dotSouthEast) + (weightWest * dotSouthWest);
	float blendNorth = (weightEast * dotNorthEast) + (weightWest * dotNorthWest);
	float blendTotal = (weightSouth * blendSouth) + (weightNorth * blendNorth);
	float noiseNegOneToOne = 1.5f * blendTotal; // 2D Perlin is in ~[-.66,.66]; map to ~[-1,1]

	return noiseNegOneToOne;
}

//-----------------------------------------------------------------------------------------------
// Perlin noise is fractal noise with "gradient vector smoothing" applied.
//
// In 2D, gradients are unit-length vectors in various directions with even angular distribution.

float Compute2dPerlinNoiseNegOneToOne(float posX, float posY, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed)
{
	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave

	float totalNoise = 0.f;
	float totalAmplitude = 0.f;
	float currentAmplitude = 1.f;
	float invScale = (1.f / scale);
	float2 currentPos = float2(posX * invScale, posY * invScale);

	for (unsigned int octaveNum = 0; octaveNum < numOctaves; ++octaveNum)
	{
		float noiseThisOctave = Compute2dPerlinNoiseNegOneToOne(currentPos.x, currentPos.y, seed);

		// Accumulate results and prepare for next octave (if any)
		totalNoise += noiseThisOctave * currentAmplitude;
		totalAmplitude += currentAmplitude;
		currentAmplitude *= octavePersistence;
		currentPos *= octaveScale;
		currentPos.x += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.y += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		++seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if (renormalize && totalAmplitude > 0.f)
	{
		totalNoise /= totalAmplitude;				// Amplitude exceeds 1.0 if octaves are used
		totalNoise = (totalNoise * 0.5f) + 0.5f;	// Map to [0,1]
		totalNoise = SmoothStep3(totalNoise);		// Push towards extents (octaves pull us away)
		totalNoise = (totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
	}

	return totalNoise;
}

float Compute2dPerlinNoiseZeroToOne(float x, float y, unsigned int seed)
{
	float noise = Compute2dPerlinNoiseNegOneToOne(x, y, seed);
	noise *= 0.5;
	noise += 0.5;
	return noise;
}

float Compute2dPerlinNoiseZeroToOne(float x, float y, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed)
{
	float noise = Compute2dPerlinNoiseNegOneToOne(x, y, scale, numOctaves, octavePersistence, octaveScale, renormalize, seed);
	noise *= 0.5;
	noise += 0.5;
	return noise;
}

//-----------------------------------------------------------------------------------------------
// Perlin noise is fractal noise with "gradient vector smoothing" applied.
//
// In 3D, gradients are unit-length vectors in random (3D) directions.
float Compute3dPerlinNoiseNegOneToOne(float x, float y, float z, float seed)
{
	const float fSQRT_3_OVER_3 = sqrt(3.0f) / 3.0f;

	const float3 gradients[8] = // Traditional "12 edges" requires modulus and isn't any better.
	{
		float3(+fSQRT_3_OVER_3, +fSQRT_3_OVER_3, +fSQRT_3_OVER_3), // Normalized unit 3D vectors
		float3(-fSQRT_3_OVER_3, +fSQRT_3_OVER_3, +fSQRT_3_OVER_3), //  pointing toward cube
		float3(+fSQRT_3_OVER_3, -fSQRT_3_OVER_3, +fSQRT_3_OVER_3), //  corners, so components
		float3(-fSQRT_3_OVER_3, -fSQRT_3_OVER_3, +fSQRT_3_OVER_3), //  are all sqrt(3)/3, i.e.
		float3(+fSQRT_3_OVER_3, +fSQRT_3_OVER_3, -fSQRT_3_OVER_3), // 0.5773502691896257645091f.
		float3(-fSQRT_3_OVER_3, +fSQRT_3_OVER_3, -fSQRT_3_OVER_3), // These are slightly better
		float3(+fSQRT_3_OVER_3, -fSQRT_3_OVER_3, -fSQRT_3_OVER_3), // than axes (1,0,0) and much
		float3(-fSQRT_3_OVER_3, -fSQRT_3_OVER_3, -fSQRT_3_OVER_3)  // faster than edges (1,1,0).
	};

	// Determine random unit "gradient vectors" for surrounding corners
	float3 cellMins = float3(floor(x), floor(y), floor(z));
	float3 cellMaxs = float3(cellMins.x + 1.f, cellMins.y + 1.f, cellMins.z + 1.f);
	int indexWestX = (int)cellMins.x;
	int indexSouthY = (int)cellMins.y;
	int indexBelowZ = (int)cellMins.z;
	int indexEastX = indexWestX + 1;
	int indexNorthY = indexSouthY + 1;
	int indexAboveZ = indexBelowZ + 1;

	unsigned int noiseBelowSW = Get3dNoiseUint(indexWestX, indexSouthY, indexBelowZ, seed);
	unsigned int noiseBelowSE = Get3dNoiseUint(indexEastX, indexSouthY, indexBelowZ, seed);
	unsigned int noiseBelowNW = Get3dNoiseUint(indexWestX, indexNorthY, indexBelowZ, seed);
	unsigned int noiseBelowNE = Get3dNoiseUint(indexEastX, indexNorthY, indexBelowZ, seed);
	unsigned int noiseAboveSW = Get3dNoiseUint(indexWestX, indexSouthY, indexAboveZ, seed);
	unsigned int noiseAboveSE = Get3dNoiseUint(indexEastX, indexSouthY, indexAboveZ, seed);
	unsigned int noiseAboveNW = Get3dNoiseUint(indexWestX, indexNorthY, indexAboveZ, seed);
	unsigned int noiseAboveNE = Get3dNoiseUint(indexEastX, indexNorthY, indexAboveZ, seed);

	float3 gradientBelowSW = gradients[noiseBelowSW & 0x00000007];
	float3 gradientBelowSE = gradients[noiseBelowSE & 0x00000007];
	float3 gradientBelowNW = gradients[noiseBelowNW & 0x00000007];
	float3 gradientBelowNE = gradients[noiseBelowNE & 0x00000007];
	float3 gradientAboveSW = gradients[noiseAboveSW & 0x00000007];
	float3 gradientAboveSE = gradients[noiseAboveSE & 0x00000007];
	float3 gradientAboveNW = gradients[noiseAboveNW & 0x00000007];
	float3 gradientAboveNE = gradients[noiseAboveNE & 0x00000007];

	// Dot each corner's gradient with displacement from corner to position
	float3 displacementFromBelowSW = float3(x - cellMins.x, y - cellMins.y, z - cellMins.z);
	float3 displacementFromBelowSE = float3(x - cellMaxs.x, y - cellMins.y, z - cellMins.z);
	float3 displacementFromBelowNW = float3(x - cellMins.x, y - cellMaxs.y, z - cellMins.z);
	float3 displacementFromBelowNE = float3(x - cellMaxs.x, y - cellMaxs.y, z - cellMins.z);
	float3 displacementFromAboveSW = float3(x - cellMins.x, y - cellMins.y, z - cellMaxs.z);
	float3 displacementFromAboveSE = float3(x - cellMaxs.x, y - cellMins.y, z - cellMaxs.z);
	float3 displacementFromAboveNW = float3(x - cellMins.x, y - cellMaxs.y, z - cellMaxs.z);
	float3 displacementFromAboveNE = float3(x - cellMaxs.x, y - cellMaxs.y, z - cellMaxs.z);

	float dotBelowSW = dot(gradientBelowSW, displacementFromBelowSW);
	float dotBelowSE = dot(gradientBelowSE, displacementFromBelowSE);
	float dotBelowNW = dot(gradientBelowNW, displacementFromBelowNW);
	float dotBelowNE = dot(gradientBelowNE, displacementFromBelowNE);
	float dotAboveSW = dot(gradientAboveSW, displacementFromAboveSW);
	float dotAboveSE = dot(gradientAboveSE, displacementFromAboveSE);
	float dotAboveNW = dot(gradientAboveNW, displacementFromAboveNW);
	float dotAboveNE = dot(gradientAboveNE, displacementFromAboveNE);

	// Do a smoothed (nonlinear) weighted average of dot results
	float weightEast = SmoothStep3(displacementFromBelowSW.x);
	float weightNorth = SmoothStep3(displacementFromBelowSW.y);
	float weightAbove = SmoothStep3(displacementFromBelowSW.z);
	float weightWest = 1.f - weightEast;
	float weightSouth = 1.f - weightNorth;
	float weightBelow = 1.f - weightAbove;

	// 8-way blend (8 -> 4 -> 2 -> 1)
	float blendBelowSouth = (weightEast * dotBelowSE) + (weightWest * dotBelowSW);
	float blendBelowNorth = (weightEast * dotBelowNE) + (weightWest * dotBelowNW);
	float blendAboveSouth = (weightEast * dotAboveSE) + (weightWest * dotAboveSW);
	float blendAboveNorth = (weightEast * dotAboveNE) + (weightWest * dotAboveNW);
	float blendBelow = (weightSouth * blendBelowSouth) + (weightNorth * blendBelowNorth);
	float blendAbove = (weightSouth * blendAboveSouth) + (weightNorth * blendAboveNorth);
	float blendTotal = (weightBelow * blendBelow) + (weightAbove * blendAbove);
	float blendNormalized = 1.66666666f * blendTotal; // 3D Perlin is ~[-.6,.6]; map to ~[-1,1]

	return blendNormalized;
}

float Compute3dPerlinNoiseNegOneToOne(float posX, float posY, float posZ, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed)
{
	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave

	float totalNoise = 0.f;
	float totalAmplitude = 0.f;
	float currentAmplitude = 1.f;
	float invScale = (1.f / scale);
	float3 currentPos = float3(posX * invScale, posY * invScale, posZ * invScale);

	for (unsigned int octaveNum = 0; octaveNum < numOctaves; ++octaveNum)
	{
		float noiseThisOctave = Compute3dPerlinNoiseNegOneToOne(currentPos.x, currentPos.y, currentPos.z, seed);

		// Accumulate results and prepare for next octave (if any)
		totalNoise += noiseThisOctave * currentAmplitude;
		totalAmplitude += currentAmplitude;
		currentAmplitude *= octavePersistence;
		currentPos *= octaveScale;
		currentPos.x += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.y += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.z += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		++seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if (renormalize && totalAmplitude > 0.f)
	{
		totalNoise /= totalAmplitude;				// Amplitude exceeds 1.0 if octaves are used
		totalNoise = (totalNoise * 0.5f) + 0.5f;	// Map to [0,1]
		totalNoise = SmoothStep3(totalNoise);		// Push towards extents (octaves pull us away)
		totalNoise = (totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
	}

	return totalNoise;
}

float Compute3dPerlinNoiseZeroToOne(float x, float y, float z, float seed)
{
	float noise = Compute3dPerlinNoiseNegOneToOne(x, y, z, seed);
	noise *= 0.5;
	noise += 0.5;
	return noise;
}

float Compute3dPerlinNoiseZeroToOne(float x, float y, float z, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed)
{
	float noise = Compute3dPerlinNoiseNegOneToOne(x, y, z, scale, numOctaves, octavePersistence, octaveScale, renormalize, seed);
	noise *= 0.5;
	noise += 0.5;
	return noise;
}

//bool slope_up = ((indexWestX / wrap) % 2) == 0;
//period = 6
//indexWestX = 3
//indexWestX / 6 = 0
//bool slopeUp = (indexWestX % 2) == 0
//if(slopeUp){
//    indexWestX % period = 3;
//}else{
//    period - (indexWestX % period)
//    6 - (8 & 6)
//}
int wrap_up_down(int n, int period)
{
    n++;
    int slope = n / period;
    bool slopeUp = (slope % 2) == 0;
    if(slopeUp){
        return n % period;
    }else{
        return period - (n % period);
    }
}

float Compute3dPerlinNoiseNegOneToOneWrapped(float x, float y, float z, unsigned int wrap, float seed)
{
	const float fSQRT_3_OVER_3 = sqrt(3.0f) / 3.0f;

     //x = fmod(x, (float)wrap);
     //y = fmod(y, (float)wrap);
     //z = fmod(z, (float)wrap);

	const float3 gradients[8] = // Traditional "12 edges" requires modulus and isn't any better.
	{
		float3(+fSQRT_3_OVER_3, +fSQRT_3_OVER_3, +fSQRT_3_OVER_3), // Normalized unit 3D vectors
		float3(-fSQRT_3_OVER_3, +fSQRT_3_OVER_3, +fSQRT_3_OVER_3), //  pointing toward cube
		float3(+fSQRT_3_OVER_3, -fSQRT_3_OVER_3, +fSQRT_3_OVER_3), //  corners, so components
		float3(-fSQRT_3_OVER_3, -fSQRT_3_OVER_3, +fSQRT_3_OVER_3), //  are all sqrt(3)/3, i.e.
		float3(+fSQRT_3_OVER_3, +fSQRT_3_OVER_3, -fSQRT_3_OVER_3), // 0.5773502691896257645091f.
		float3(-fSQRT_3_OVER_3, +fSQRT_3_OVER_3, -fSQRT_3_OVER_3), // These are slightly better
		float3(+fSQRT_3_OVER_3, -fSQRT_3_OVER_3, -fSQRT_3_OVER_3), // than axes (1,0,0) and much
		float3(-fSQRT_3_OVER_3, -fSQRT_3_OVER_3, -fSQRT_3_OVER_3)  // faster than edges (1,1,0).
	};

	// Determine random unit "gradient vectors" for surrounding corners
	float3 cellMins = float3(floor(x), floor(y), floor(z));
	float3 cellMaxs = float3(cellMins.x + 1.f, cellMins.y + 1.f, cellMins.z + 1.f);

	int indexWestX = (int)cellMins.x;
	int indexSouthY = (int)cellMins.y;
	int indexBelowZ = (int)cellMins.z;
	int indexEastX = indexWestX + 1;
	int indexNorthY = indexSouthY + 1;
	int indexAboveZ = indexBelowZ + 1;

    //bool slope_up = ((indexWestX / wrap) % 2) == 0;
    //period = 6
    //indexWestX = 3
    //indexWestX / 6 = 0
    //bool slopeUp = (indexWestX % 2) == 0
    //if(slopeUp){
    //    indexWestX % period = 3;
    //}else{
    //    period - (indexWestX % period)
    //    6 - (8 & 6)
    //}

	//indexWestX  = wrap_up_down(indexWestX, wrap);
	//indexSouthY = wrap_up_down(indexSouthY, wrap);
	//indexBelowZ = wrap_up_down(indexBelowZ, wrap);
	//indexEastX  = wrap_up_down(indexEastX, wrap);
	//indexNorthY = wrap_up_down(indexNorthY, wrap);
	//indexAboveZ = wrap_up_down(indexAboveZ, wrap);

	indexWestX = (indexWestX + 1) % wrap;
	indexSouthY = (indexSouthY + 1) % wrap;
	indexBelowZ = (indexBelowZ + 1) % wrap;
	indexEastX = (indexEastX + 1) % wrap;
	indexNorthY = (indexNorthY + 1) % wrap;
	indexAboveZ = (indexAboveZ + 1) % wrap;

	unsigned int noiseBelowSW = Get3dNoiseUint(indexWestX, indexSouthY, indexBelowZ, seed);
	unsigned int noiseBelowSE = Get3dNoiseUint(indexEastX, indexSouthY, indexBelowZ, seed);
	unsigned int noiseBelowNW = Get3dNoiseUint(indexWestX, indexNorthY, indexBelowZ, seed);
	unsigned int noiseBelowNE = Get3dNoiseUint(indexEastX, indexNorthY, indexBelowZ, seed);
	unsigned int noiseAboveSW = Get3dNoiseUint(indexWestX, indexSouthY, indexAboveZ, seed);
	unsigned int noiseAboveSE = Get3dNoiseUint(indexEastX, indexSouthY, indexAboveZ, seed);
	unsigned int noiseAboveNW = Get3dNoiseUint(indexWestX, indexNorthY, indexAboveZ, seed);
	unsigned int noiseAboveNE = Get3dNoiseUint(indexEastX, indexNorthY, indexAboveZ, seed);

	float3 gradientBelowSW = gradients[noiseBelowSW & 0x00000007];
	float3 gradientBelowSE = gradients[noiseBelowSE & 0x00000007];
	float3 gradientBelowNW = gradients[noiseBelowNW & 0x00000007];
	float3 gradientBelowNE = gradients[noiseBelowNE & 0x00000007];
	float3 gradientAboveSW = gradients[noiseAboveSW & 0x00000007];
	float3 gradientAboveSE = gradients[noiseAboveSE & 0x00000007];
	float3 gradientAboveNW = gradients[noiseAboveNW & 0x00000007];
	float3 gradientAboveNE = gradients[noiseAboveNE & 0x00000007];

	// Dot each corner's gradient with displacement from corner to position
	float3 displacementFromBelowSW = float3(x - cellMins.x, y - cellMins.y, z - cellMins.z);
	float3 displacementFromBelowSE = float3(x - cellMaxs.x, y - cellMins.y, z - cellMins.z);
	float3 displacementFromBelowNW = float3(x - cellMins.x, y - cellMaxs.y, z - cellMins.z);
	float3 displacementFromBelowNE = float3(x - cellMaxs.x, y - cellMaxs.y, z - cellMins.z);
	float3 displacementFromAboveSW = float3(x - cellMins.x, y - cellMins.y, z - cellMaxs.z);
	float3 displacementFromAboveSE = float3(x - cellMaxs.x, y - cellMins.y, z - cellMaxs.z);
	float3 displacementFromAboveNW = float3(x - cellMins.x, y - cellMaxs.y, z - cellMaxs.z);
	float3 displacementFromAboveNE = float3(x - cellMaxs.x, y - cellMaxs.y, z - cellMaxs.z);

	float dotBelowSW = dot(gradientBelowSW, displacementFromBelowSW);
	float dotBelowSE = dot(gradientBelowSE, displacementFromBelowSE);
	float dotBelowNW = dot(gradientBelowNW, displacementFromBelowNW);
	float dotBelowNE = dot(gradientBelowNE, displacementFromBelowNE);
	float dotAboveSW = dot(gradientAboveSW, displacementFromAboveSW);
	float dotAboveSE = dot(gradientAboveSE, displacementFromAboveSE);
	float dotAboveNW = dot(gradientAboveNW, displacementFromAboveNW);
	float dotAboveNE = dot(gradientAboveNE, displacementFromAboveNE);

	// Do a smoothed (nonlinear) weighted average of dot results
	float weightEast = SmoothStep3(displacementFromBelowSW.x);
	float weightNorth = SmoothStep3(displacementFromBelowSW.y);
	float weightAbove = SmoothStep3(displacementFromBelowSW.z);
	float weightWest = 1.f - weightEast;
	float weightSouth = 1.f - weightNorth;
	float weightBelow = 1.f - weightAbove;

	// 8-way blend (8 -> 4 -> 2 -> 1)
	float blendBelowSouth = (weightEast * dotBelowSE) + (weightWest * dotBelowSW);
	float blendBelowNorth = (weightEast * dotBelowNE) + (weightWest * dotBelowNW);
	float blendAboveSouth = (weightEast * dotAboveSE) + (weightWest * dotAboveSW);
	float blendAboveNorth = (weightEast * dotAboveNE) + (weightWest * dotAboveNW);
	float blendBelow = (weightSouth * blendBelowSouth) + (weightNorth * blendBelowNorth);
	float blendAbove = (weightSouth * blendAboveSouth) + (weightNorth * blendAboveNorth);
	float blendTotal = (weightBelow * blendBelow) + (weightAbove * blendAbove);
	float blendNormalized = 1.66666666f * blendTotal; // 3D Perlin is ~[-.6,.6]; map to ~[-1,1]

	return blendNormalized;
}

float Compute3dPerlinNoiseNegOneToOneWrapped(float posX, float posY, float posZ, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, int wrap, bool renormalize, unsigned int seed)
{
	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave

	float totalNoise = 0.f;
	float totalAmplitude = 0.f;
	float currentAmplitude = 1.f;
	float invScale = (1.f / scale);
	float3 currentPos = float3(posX * invScale, posY * invScale, posZ * invScale);

	for (unsigned int octaveNum = 0; octaveNum < numOctaves; ++octaveNum)
	{
		float noiseThisOctave = Compute3dPerlinNoiseNegOneToOneWrapped(currentPos.x, currentPos.y, currentPos.z, wrap, seed);

		// Accumulate results and prepare for next octave (if any)
		totalNoise += noiseThisOctave * currentAmplitude;
		totalAmplitude += currentAmplitude;
		currentAmplitude *= octavePersistence;
		currentPos *= octaveScale;
		currentPos.x += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.y += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.z += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		++seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if (renormalize && totalAmplitude > 0.f)
	{
		totalNoise /= totalAmplitude;				// Amplitude exceeds 1.0 if octaves are used
		totalNoise = (totalNoise * 0.5f) + 0.5f;	// Map to [0,1]
		totalNoise = SmoothStep3(totalNoise);		// Push towards extents (octaves pull us away)
		totalNoise = (totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
	}

	return totalNoise;
}

float Compute3dPerlinNoiseZeroToOneWrapped(float x, float y, float z, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, int wrap, bool renormalize, unsigned int seed)
{
	float noise = Compute3dPerlinNoiseNegOneToOneWrapped(x, y, z, scale, numOctaves, octavePersistence, octaveScale, wrap, renormalize, seed);
	noise *= 0.5;
	noise += 0.5;
	return noise;
}

//-----------------------------------------------------------------------------------------------
// Perlin noise is fractal noise with "gradient vector smoothing" applied.
//
// In 4D, gradients are unit-length hyper-vectors in random (4D) directions.

float Compute4dPerlinNoiseNegOneToOne(float x, float y, float z, float t, unsigned int seed)
{
	const float4 gradients[16] = // Hard to tell if this is any better in 4D than just having 8
	{
		float4(+0.5f, +0.5f, +0.5f, +0.5f), // Normalized unit 4D vectors pointing toward each
		float4(-0.5f, +0.5f, +0.5f, +0.5f), //  of the 16 hypercube corners, so components are
		float4(+0.5f, -0.5f, +0.5f, +0.5f), //  all sqrt(4)/4, i.e. one-half.
		float4(-0.5f, -0.5f, +0.5f, +0.5f), //
		float4(+0.5f, +0.5f, -0.5f, +0.5f), // It's hard to tell whether these are any better
		float4(-0.5f, +0.5f, -0.5f, +0.5f), //  or worse than vectors facing axes (1,0,0,0) or
		float4(+0.5f, -0.5f, -0.5f, +0.5f), //  3D edges (.7,.7,0,0) or 4D edges (.57,.57,.57,0)
		float4(-0.5f, -0.5f, -0.5f, +0.5f), //  but less-axial gradients looked a little better
		float4(+0.5f, +0.5f, +0.5f, -0.5f), //  with 2D and 3D noise so I'm assuming this is as
		float4(-0.5f, +0.5f, +0.5f, -0.5f), //  good or better as any other gradient-selection
		float4(+0.5f, -0.5f, +0.5f, -0.5f), //  scheme (and is crazy-fast).  *shrug*
		float4(-0.5f, -0.5f, +0.5f, -0.5f), //
		float4(+0.5f, +0.5f, -0.5f, -0.5f), // Plus, we want a power-of-two number of evenly-
		float4(-0.5f, +0.5f, -0.5f, -0.5f), //  distributed gradients, so we can cheaply select
		float4(+0.5f, -0.5f, -0.5f, -0.5f), //  one from bit-noise (use bit-mask, not modulus).
		float4(-0.5f, -0.5f, -0.5f, -0.5f)  //
	};

	// Determine random unit "gradient vectors" for 16 surrounding 4D (hypercube) cell corners
	float4 cellMins = float4(floor(x), floor(y), floor(z), floor(t));
	float4 cellMaxs = float4(cellMins.x + 1.f, cellMins.y + 1.f, cellMins.z + 1.f, cellMins.w + 1.f);
	int indexWestX = (int)cellMins.x;
	int indexSouthY = (int)cellMins.y;
	int indexBelowZ = (int)cellMins.z;
	int indexBeforeT = (int)cellMins.w;
	int indexEastX = indexWestX + 1;
	int indexNorthY = indexSouthY + 1;
	int indexAboveZ = indexBelowZ + 1;
	int indexAfterT = indexBeforeT + 1;

	// "BeforeBSW" stands for "BeforeBelowSouthWest" below (i.e. 4D hypercube mins), etc.
	unsigned int noiseBeforeBSW = Get4dNoiseUint(indexWestX, indexSouthY, indexBelowZ, indexBeforeT, seed);
	unsigned int noiseBeforeBSE = Get4dNoiseUint(indexEastX, indexSouthY, indexBelowZ, indexBeforeT, seed);
	unsigned int noiseBeforeBNW = Get4dNoiseUint(indexWestX, indexNorthY, indexBelowZ, indexBeforeT, seed);
	unsigned int noiseBeforeBNE = Get4dNoiseUint(indexEastX, indexNorthY, indexBelowZ, indexBeforeT, seed);
	unsigned int noiseBeforeASW = Get4dNoiseUint(indexWestX, indexSouthY, indexAboveZ, indexBeforeT, seed);
	unsigned int noiseBeforeASE = Get4dNoiseUint(indexEastX, indexSouthY, indexAboveZ, indexBeforeT, seed);
	unsigned int noiseBeforeANW = Get4dNoiseUint(indexWestX, indexNorthY, indexAboveZ, indexBeforeT, seed);
	unsigned int noiseBeforeANE = Get4dNoiseUint(indexEastX, indexNorthY, indexAboveZ, indexBeforeT, seed);
	unsigned int noiseAfterBSW = Get4dNoiseUint(indexWestX, indexSouthY, indexBelowZ, indexAfterT, seed);
	unsigned int noiseAfterBSE = Get4dNoiseUint(indexEastX, indexSouthY, indexBelowZ, indexAfterT, seed);
	unsigned int noiseAfterBNW = Get4dNoiseUint(indexWestX, indexNorthY, indexBelowZ, indexAfterT, seed);
	unsigned int noiseAfterBNE = Get4dNoiseUint(indexEastX, indexNorthY, indexBelowZ, indexAfterT, seed);
	unsigned int noiseAfterASW = Get4dNoiseUint(indexWestX, indexSouthY, indexAboveZ, indexAfterT, seed);
	unsigned int noiseAfterASE = Get4dNoiseUint(indexEastX, indexSouthY, indexAboveZ, indexAfterT, seed);
	unsigned int noiseAfterANW = Get4dNoiseUint(indexWestX, indexNorthY, indexAboveZ, indexAfterT, seed);
	unsigned int noiseAfterANE = Get4dNoiseUint(indexEastX, indexNorthY, indexAboveZ, indexAfterT, seed);

	// Mask with 15 (mod 16) to look up in gradients table
	float4 gradientBeforeBSW = gradients[noiseBeforeBSW & 0x0000000F];
	float4 gradientBeforeBSE = gradients[noiseBeforeBSE & 0x0000000F];
	float4 gradientBeforeBNW = gradients[noiseBeforeBNW & 0x0000000F];
	float4 gradientBeforeBNE = gradients[noiseBeforeBNE & 0x0000000F];
	float4 gradientBeforeASW = gradients[noiseBeforeASW & 0x0000000F];
	float4 gradientBeforeASE = gradients[noiseBeforeASE & 0x0000000F];
	float4 gradientBeforeANW = gradients[noiseBeforeANW & 0x0000000F];
	float4 gradientBeforeANE = gradients[noiseBeforeANE & 0x0000000F];
	float4 gradientAfterBSW = gradients[noiseAfterBSW & 0x0000000F];
	float4 gradientAfterBSE = gradients[noiseAfterBSE & 0x0000000F];
	float4 gradientAfterBNW = gradients[noiseAfterBNW & 0x0000000F];
	float4 gradientAfterBNE = gradients[noiseAfterBNE & 0x0000000F];
	float4 gradientAfterASW = gradients[noiseAfterASW & 0x0000000F];
	float4 gradientAfterASE = gradients[noiseAfterASE & 0x0000000F];
	float4 gradientAfterANW = gradients[noiseAfterANW & 0x0000000F];
	float4 gradientAfterANE = gradients[noiseAfterANE & 0x0000000F];

	// Dot each corner's gradient with displacement from corner to position
	float4 displacementFromBeforeBSW = float4(x - cellMins.x, y - cellMins.y, z - cellMins.z, t - cellMins.w);
	float4 displacementFromBeforeBSE = float4(x - cellMaxs.x, y - cellMins.y, z - cellMins.z, t - cellMins.w);
	float4 displacementFromBeforeBNW = float4(x - cellMins.x, y - cellMaxs.y, z - cellMins.z, t - cellMins.w);
	float4 displacementFromBeforeBNE = float4(x - cellMaxs.x, y - cellMaxs.y, z - cellMins.z, t - cellMins.w);
	float4 displacementFromBeforeASW = float4(x - cellMins.x, y - cellMins.y, z - cellMaxs.z, t - cellMins.w);
	float4 displacementFromBeforeASE = float4(x - cellMaxs.x, y - cellMins.y, z - cellMaxs.z, t - cellMins.w);
	float4 displacementFromBeforeANW = float4(x - cellMins.x, y - cellMaxs.y, z - cellMaxs.z, t - cellMins.w);
	float4 displacementFromBeforeANE = float4(x - cellMaxs.x, y - cellMaxs.y, z - cellMaxs.z, t - cellMins.w);
	float4 displacementFromAfterBSW = float4(x - cellMins.x, y - cellMins.y, z - cellMins.z, t - cellMaxs.w);
	float4 displacementFromAfterBSE = float4(x - cellMaxs.x, y - cellMins.y, z - cellMins.z, t - cellMaxs.w);
	float4 displacementFromAfterBNW = float4(x - cellMins.x, y - cellMaxs.y, z - cellMins.z, t - cellMaxs.w);
	float4 displacementFromAfterBNE = float4(x - cellMaxs.x, y - cellMaxs.y, z - cellMins.z, t - cellMaxs.w);
	float4 displacementFromAfterASW = float4(x - cellMins.x, y - cellMins.y, z - cellMaxs.z, t - cellMaxs.w);
	float4 displacementFromAfterASE = float4(x - cellMaxs.x, y - cellMins.y, z - cellMaxs.z, t - cellMaxs.w);
	float4 displacementFromAfterANW = float4(x - cellMins.x, y - cellMaxs.y, z - cellMaxs.z, t - cellMaxs.w);
	float4 displacementFromAfterANE = float4(x - cellMaxs.x, y - cellMaxs.y, z - cellMaxs.z, t - cellMaxs.w);

	float dotBeforeBSW = dot(gradientBeforeBSW, displacementFromBeforeBSW);
	float dotBeforeBSE = dot(gradientBeforeBSE, displacementFromBeforeBSE);
	float dotBeforeBNW = dot(gradientBeforeBNW, displacementFromBeforeBNW);
	float dotBeforeBNE = dot(gradientBeforeBNE, displacementFromBeforeBNE);
	float dotBeforeASW = dot(gradientBeforeASW, displacementFromBeforeASW);
	float dotBeforeASE = dot(gradientBeforeASE, displacementFromBeforeASE);
	float dotBeforeANW = dot(gradientBeforeANW, displacementFromBeforeANW);
	float dotBeforeANE = dot(gradientBeforeANE, displacementFromBeforeANE);
	float dotAfterBSW = dot(gradientAfterBSW, displacementFromAfterBSW);
	float dotAfterBSE = dot(gradientAfterBSE, displacementFromAfterBSE);
	float dotAfterBNW = dot(gradientAfterBNW, displacementFromAfterBNW);
	float dotAfterBNE = dot(gradientAfterBNE, displacementFromAfterBNE);
	float dotAfterASW = dot(gradientAfterASW, displacementFromAfterASW);
	float dotAfterASE = dot(gradientAfterASE, displacementFromAfterASE);
	float dotAfterANW = dot(gradientAfterANW, displacementFromAfterANW);
	float dotAfterANE = dot(gradientAfterANE, displacementFromAfterANE);

	// Do a smoothed (nonlinear) weighted average of dot results
	float weightEast = SmoothStep3(displacementFromBeforeBSW.x);
	float weightNorth = SmoothStep3(displacementFromBeforeBSW.y);
	float weightAbove = SmoothStep3(displacementFromBeforeBSW.z);
	float weightAfter = SmoothStep3(displacementFromBeforeBSW.w);
	float weightWest = 1.f - weightEast;
	float weightSouth = 1.f - weightNorth;
	float weightBelow = 1.f - weightAbove;
	float weightBefore = 1.f - weightAfter;

	// 16-way blend (16 -> 8 -> 4 -> 2 -> 1)
	float blendBeforeBelowSouth = (weightEast * dotBeforeBSE) + (weightWest * dotBeforeBSW);
	float blendBeforeBelowNorth = (weightEast * dotBeforeBNE) + (weightWest * dotBeforeBNW);
	float blendBeforeAboveSouth = (weightEast * dotBeforeASE) + (weightWest * dotBeforeASW);
	float blendBeforeAboveNorth = (weightEast * dotBeforeANE) + (weightWest * dotBeforeANW);
	float blendAfterBelowSouth = (weightEast * dotAfterBSE) + (weightWest * dotAfterBSW);
	float blendAfterBelowNorth = (weightEast * dotAfterBNE) + (weightWest * dotAfterBNW);
	float blendAfterAboveSouth = (weightEast * dotAfterASE) + (weightWest * dotAfterASW);
	float blendAfterAboveNorth = (weightEast * dotAfterANE) + (weightWest * dotAfterANW);
	float blendBeforeBelow = (weightSouth * blendBeforeBelowSouth) + (weightNorth * blendBeforeBelowNorth);
	float blendBeforeAbove = (weightSouth * blendBeforeAboveSouth) + (weightNorth * blendBeforeAboveNorth);
	float blendAfterBelow = (weightSouth * blendAfterBelowSouth) + (weightNorth * blendAfterBelowNorth);
	float blendAfterAbove = (weightSouth * blendAfterAboveSouth) + (weightNorth * blendAfterAboveNorth);
	float blendBefore = (weightBelow * blendBeforeBelow) + (weightAbove * blendBeforeAbove);
	float blendAfter = (weightBelow * blendAfterBelow) + (weightAbove * blendAfterAbove);
	float blendTotal = (weightBefore * blendBefore) + (weightAfter * blendAfter);
	float noiseNegOneToOne = 1.6f * blendTotal; // 4D Perlin is in ~[-.5,.5]; map to ~[-1,1]

	return noiseNegOneToOne;
}

float Compute4dPerlinNoiseNegOneToOne(float posX, float posY, float posZ, float posT, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed)
{
	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave

	float totalNoise = 0.f;
	float totalAmplitude = 0.f;
	float currentAmplitude = 1.f;
	float invScale = (1.f / scale);
	float4 currentPos = float4(posX * invScale, posY * invScale, posZ * invScale, posT * invScale);

	for (unsigned int octaveNum = 0; octaveNum < numOctaves; ++octaveNum)
	{
		float noiseThisOctave = Compute4dPerlinNoiseNegOneToOne(currentPos.x, currentPos.y, currentPos.z, currentPos.w, seed);

		// Accumulate results and prepare for next octave (if any)
		totalNoise += noiseThisOctave * currentAmplitude;
		totalAmplitude += currentAmplitude;
		currentAmplitude *= octavePersistence;
		currentPos *= octaveScale;
		currentPos.x += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.y += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.z += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.w += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		++seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if (renormalize && totalAmplitude > 0.f)
	{
		totalNoise /= totalAmplitude;				// Amplitude exceeds 1.0 if octaves are used
		totalNoise = (totalNoise * 0.5f) + 0.5f;	// Map to [0,1]
		totalNoise = SmoothStep3(totalNoise);		// Push towards extents (octaves pull us away)
		totalNoise = (totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
	}

	return totalNoise;
}

float Compute4dPerlinNoiseZeroToOne(float x, float y, float z, float t, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed)
{
	float noise = Compute4dPerlinNoiseNegOneToOne(x, y, z, t, scale, numOctaves, octavePersistence, octaveScale, renormalize, seed);
	noise *= 0.5;
	noise += 0.5;
	return noise;
}

float2 random2(float2 p)
{
     float x = dot(p, float2(127.1,311.7));
     float y = dot(p, float2(269.5,183.3));
     float2 v = float2(x, y);
     return frac(sin(v) * 43758.5453);
}

float3 random3(float3 p)
{
    float x = dot(p, float3(127.1f, 311.7f, 204.3f));
    float y = dot(p, float3(269.5f, 183.3f, 317.9f));
    float z = dot(p, float3(334.7f, 113.4f, 253.1f));
    float3 sin_vec = float3(sin(x), sin(y), sin(z));
    sin_vec *= 43758.5453f;
    float3 ret;
    ret.x = frac(sin_vec.x);
    ret.y = frac(sin_vec.y);
    ret.z = frac(sin_vec.z);
    return ret;
}

// I tried implementing my Perlin noise myself using http://flafla2.github.io/2014/08/09/perlinnoise.html
// It was okay, but I decided to fall back to Squirrel's implementation because its just better.
#if 0
// Randomized list of integers from 0 -> 255, but repeated twice
// Its repeated because when hasing into this list you could potentially
// add 255 & 255 together
static const int p[512] = {
   151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
   8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
   35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,
   134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
   55,46,245,40,244,102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,
   18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,
   250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
   189,28,42,223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167,
   43,172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,
   97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
   49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
   151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
   8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
   35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,
   134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
   55,46,245,40,244,102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,
   18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,
   250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
   189,28,42,223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167,
   43,172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,
   97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
   49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

int FastFloorToInt(float val){
	int valAsInt = (int)val;

	// If val is positive or val is a whole number then just return it casted as an int
	if (val > 0.f || (val - (float)valAsInt) == 0.f)
		return valAsInt;

	// Its a negative fractional number so round down towards negative infinity
	return valAsInt - 1;
}

float Lerp(float a, float b, float t) {
	return a + (t * (b - a));
}

// Source: http://riven8192.blogspot.com/2010/08/calculate-perlinnoise-twice-as-fast.html
// The "hash" is a random number picked from the permutation array based on input to Perlin
//
// The gradient vectors are the following:
// (1,1,0),(-1,1,0),(1,-1,0),(-1,-1,0),
// (1,0,1),(-1,0,1),(1,0,-1),(-1,0,-1),
// (0,1,1),(0,-1,1),(0,1,-1),(0,-1,-1)
//
// Doing the dot product with those will always result in just combining x,y,z in different ways
float ApplyGradient(int hash, float x, float y, float z)
{
    switch(hash & 0xF)
    {
        case 0x0: return  x + y;
        case 0x1: return -x + y;
        case 0x2: return  x - y;
        case 0x3: return -x - y;
        case 0x4: return  x + z;
        case 0x5: return -x + z;
        case 0x6: return  x - z;
        case 0x7: return -x - z;
        case 0x8: return  y + z;
        case 0x9: return -y + z;
        case 0xA: return  y - z;
        case 0xB: return -y - z;
        case 0xC: return  y + x;
        case 0xD: return -y + z;
        case 0xE: return  y - x;
        case 0xF: return -y - z;
        default: return 0; // never happens
    }
}

// Source: http://mrl.nyu.edu/~perlin/noise/
float PerlinNoise(float x, float y, float z)
{
	// Get the unit cube coordinates (the integer parts of x,y,z from 0 - 255)
	int xi = FastFloorToInt(x) & 255;
	int yi = FastFloorToInt(y) & 255;
	int zi = FastFloorToInt(z) & 255;

	// Get the coordinates within that unit cube (the fractional parts of x,y,z)
	x = frac(x);
	y = frac(y);
	z = frac(z);

	// Used as the hash values for getting gradients and dot'ing with them
	int A = p[xi] + yi;
	int AA = p[A] + zi;
	int AB = p[A + 1] + zi;

	int B = p[xi + 1] + yi;
	int BA = p[B] + zi;
	int BB = p[B + 1] + zi;

	// Apply gradients for each corner of our unit cube
	float x0y0z0 = ApplyGradient(p[AA], x,			y,		z);
	float x1y0z0 = ApplyGradient(p[BA], x - 1,		y,		z);

	float x0y1z0 = ApplyGradient(p[AB], x,			y - 1,	z);
	float x1y1z0 = ApplyGradient(p[BB], x - 1,		y - 1,	z);

	float x0y0z1 = ApplyGradient(p[AA + 1], x,			y,		z - 1);
	float x1y0z1 = ApplyGradient(p[BA + 1], x - 1,		y,		z - 1);

	float x0y1z1 = ApplyGradient(p[AB + 1], x,			y - 1,	z - 1);
	float x1y1z1 = ApplyGradient(p[BB + 1], x - 1,		y - 1,	z - 1);

	// BUT WILL IT BLEND!?
	float x_smooth = SmoothStep5(x);
	float y_smooth = SmoothStep5(y);
	float z_smooth = SmoothStep5(z);

	float x0_Blend = Lerp(x0y0z0, x1y0z0, x_smooth);
	float x1_Blend = Lerp(x0y1z0, x1y1z0, x_smooth);
	float x2_Blend = Lerp(x0y0z1, x1y0z1, x_smooth);
	float x3_Blend = Lerp(x0y1z1, x1y1z1, x_smooth);

	float y0_Blend = Lerp(x0_Blend, x1_Blend, y_smooth);
	float y1_Blend = Lerp(x2_Blend, x3_Blend, y_smooth);

	float finalBlend = Lerp(y0_Blend, y1_Blend, z_smooth);

	return finalBlend;
}

float PerlinNoise(float x, float y, float z, float numOctaves, float persistence)
{
	float total = 0;
	float maxValue = 0;
	float amplitude = 1;
	float frequency = 1;
	for (int octave = 0; octave < numOctaves; ++octave){
		total += PerlinNoise_Stephen(x * frequency, y * frequency, z * frequency) * amplitude;

		maxValue += amplitude;

		amplitude *= persistence;
		frequency *= 2;
	}

	// Renormalize
	return total / maxValue;
}
#endif

#endif


float get_worley_noise_2d(float2 in_pos)
{
    float2 i_st = floor(in_pos);
    float2 f_st = frac(in_pos);

    float m_dist = 1.0;  // minimun distance
    for (int y= -1; y <= 1; y++) {
        for (int x= -1; x <= 1; x++) {
            // Neighbor place in the grid
            float2 neighbor = float2((float)x, (float)y);

            // Random position from current + neighbor place in the grid
            float2 p = random2(i_st + neighbor);

			// Vector between the pixel and the point
            float2 diff = neighbor + p - f_st;

            // Distance to the point
            float dist = length(diff);

            // Keep the closer distance
            m_dist = min(m_dist, dist);
        }
    }

    return m_dist;
}

float get_worley_noise_2d(float2 position, float scale, unsigned int num_octaves, float octave_persistence, float octave_scale, bool renormalize)
{
	float total_noise = 0.f;
	float total_amplitude = 0.f;
	float current_amplitude = 1.f;
    float2 current_position = position * scale;

	for (unsigned int octave = 0; octave < num_octaves; ++octave)
	{
        float noise_this_octave = get_worley_noise_2d(current_position);

		// Accumulate results and prepare for next octave (if any)
		total_noise += noise_this_octave * current_amplitude;
		total_amplitude += current_amplitude;
		current_amplitude *= octave_persistence;
		current_position *= octave_scale;
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if (renormalize && total_amplitude > 0.f)
	{
		total_noise /= total_amplitude;				// Amplitude exceeds 1.0 if octaves are used
		total_noise = (total_noise * 0.5f) + 0.5f;	// Map to [0,1]
		total_noise = SmoothStep3(total_noise);		// Push towards extents (octaves pull us away)
		total_noise = (total_noise * 2.0f) - 1.f;		// Map back to [-1,1]
	}

	return total_noise;
}

//-----------------------------------------------------------------------------------------------
float get_worley_noise_3d(float3 position)
{
    float x_int = floor(position.x);
    float x_frac = frac(position.x);

    float y_int = floor(position.y);
    float y_frac = frac(position.y);

    float z_int = floor(position.z);
    float z_frac = frac(position.z);

    float3 i_st = float3(x_int, y_int, z_int);
    float3 f_st = float3(x_frac, y_frac, z_frac);

    float m_dist = 1.0;  // minimun distance
    for(int z = -1; z <= 1; z++){
        for(int y = -1; y <= 1; y++) {
            for(int x = -1; x <= 1; x++) {
                // Neighbor place in the grid
                float3 neighbor = float3((float)x, (float)y, (float)z);

                // Random position from current + neighbor place in the grid
                float3 p = random3(i_st + neighbor);

                // Vector between the pixel and the point
                float3 diff = neighbor + p - f_st;

                // Distance to the point
                float dist = length(diff);

                // Keep the closer distance
                m_dist = min(m_dist, dist);
            }
        }
    }

    return m_dist;
}

float get_worley_noise_3d(float x_pos, float y_pos, float z_pos)
{
    return get_worley_noise_3d(float3(x_pos, y_pos, z_pos));
}

float get_worley_noise_3d(float3 position, float scale, unsigned int num_octaves, float octave_persistence, float octave_scale, bool renormalize)
{
    float total_noise = 0.f;
    float total_amplitude = 0.f;
    float current_amplitude = 1.f;
    float3 current_position = position * scale;

    for(unsigned int octave = 0; octave < num_octaves; ++octave)
    {
        float noise_this_octave = get_worley_noise_3d(current_position.x, current_position.y, current_position.z);

        // Accumulate results and prepare for next octave (if any)
        total_noise += noise_this_octave * current_amplitude;
        total_amplitude += current_amplitude;
        current_amplitude *= octave_persistence;
        current_position *= octave_scale;
    }

    // Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
    if(renormalize && total_amplitude > 0.f)
    {
        total_noise /= total_amplitude;				// Amplitude exceeds 1.0 if octaves are used
        total_noise = (total_noise * 0.5f) + 0.5f;	// Map to [0,1]
        total_noise = SmoothStep3(total_noise);		// Push towards extents (octaves pull us away)
        total_noise = (total_noise * 2.0f) - 1.f;		// Map back to [-1,1]
    }

    return total_noise;
}

//-----------------------------------------------------------------------------------------------
float get_worley_noise_3d(float x, float y, float z, float scale, unsigned int num_octaves, float octave_persistence, float octave_scale, bool renormalize)
{
    return get_worley_noise_3d(float3(x, y, z), scale, num_octaves, octave_persistence, octave_scale, renormalize);
}