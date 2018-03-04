//-----------------------------------------------------------------------------------------------
// Noise.cpp
//
//#include "GypsyPrecompiledHeader.hpp"

//-----------------------------------------------------------------------------------------------
#include "Engine/Math/Noise.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/EasingFuncs.hpp"
#include <math.h>


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


/////////////////////////////////////////////////////////////////////////////////////////////////
// For all fractal (and Perlin) noise functions, the following internal naming conventions
//	are used, primarily to help me visualize 3D and 4D constructs clearly.  They need not
//	have any actual bearing on / relationship to actual external coordinate systems.
//
// 1D noise: only X (+east / -west)
// 2D noise: also Y (+north / -south)
// 3D noise: also Z (+above / -below)
// 4D noise: also T (+after / -before)
/////////////////////////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------------------------
float Compute1dFractalNoise( float position, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed )
{
	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave

	float totalNoise = 0.f;
	float totalAmplitude = 0.f;
	float currentAmplitude = 1.f;
	float currentPosition = position * (1.f / scale);

	for( unsigned int octaveNum = 0; octaveNum < numOctaves; ++ octaveNum )
	{
		// Determine noise values at nearby integer "grid point" positions
		float positionFloor = floorf( currentPosition );
		int indexWest = (int) positionFloor;
		int indexEast = indexWest + 1;
		float valueWest = Get1dNoiseZeroToOne( indexWest, seed );
		float valueEast = Get1dNoiseZeroToOne( indexEast, seed );

		// Do a smoothed (nonlinear) weighted average of nearby grid point values
		float distanceFromWest = currentPosition - positionFloor;
		float weightEast = SmoothStep3( distanceFromWest ); // Gives rounder (nonlinear) results
		float weightWest = 1.f - weightEast;
		float noiseZeroToOne = (valueWest * weightWest) + (valueEast * weightEast);
		float noiseThisOctave = 2.f * (noiseZeroToOne - 0.5f); // Map from [0,1] to [-1,1]

		// Accumulate results and prepare for next octave (if any)
		totalNoise += noiseThisOctave * currentAmplitude;
		totalAmplitude += currentAmplitude;
		currentAmplitude *= octavePersistence;
		currentPosition *= octaveScale;
		currentPosition += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		++ seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if( renormalize && totalAmplitude > 0.f )
	{
		totalNoise /= totalAmplitude;				// Amplitude exceeds 1.0 if octaves are used!
		totalNoise = (totalNoise * 0.5f) + 0.5f;	// Map to [0,1]
		totalNoise = SmoothStep3( totalNoise );		// Push towards extents (octaves pull us away)
		totalNoise = (totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
	}

	return totalNoise;
}


//-----------------------------------------------------------------------------------------------
float Compute2dFractalNoise( float posX, float posY, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed )
{
	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave

	float totalNoise = 0.f;
	float totalAmplitude = 0.f;
	float currentAmplitude = 1.f;
	float invScale = (1.f / scale);
	Vector2 currentPos( posX * invScale, posY * invScale );

	for( unsigned int octaveNum = 0; octaveNum < numOctaves; ++ octaveNum )
	{
		// Determine noise values at nearby integer "grid point" positions
		Vector2 cellMins( floorf( currentPos.x ), floorf( currentPos.y ) );
		int indexWestX = (int) cellMins.x;
		int indexSouthY = (int) cellMins.y;
		int indexEastX = indexWestX + 1;
		int indexNorthY = indexSouthY + 1;
		float valueSouthWest = Get2dNoiseZeroToOne( indexWestX, indexSouthY, seed );
		float valueSouthEast = Get2dNoiseZeroToOne( indexEastX, indexSouthY, seed );
		float valueNorthWest = Get2dNoiseZeroToOne( indexWestX, indexNorthY, seed );
		float valueNorthEast = Get2dNoiseZeroToOne( indexEastX, indexNorthY, seed );

		// Do a smoothed (nonlinear) weighted average of nearby grid point values
		Vector2 displacementFromMins = currentPos - cellMins;
		float weightEast  = SmoothStep3( displacementFromMins.x );
		float weightNorth = SmoothStep3( displacementFromMins.y );
		float weightWest  = 1.f - weightEast;
		float weightSouth = 1.f - weightNorth;

		float blendSouth = (weightEast * valueSouthEast) + (weightWest * valueSouthWest);
		float blendNorth = (weightEast * valueNorthEast) + (weightWest * valueNorthWest);
		float blendTotal = (weightSouth * blendSouth) + (weightNorth * blendNorth);
		float noiseThisOctave = 2.f * (blendTotal - 0.5f); // Map from [0,1] to [-1,1]

		// Accumulate results and prepare for next octave (if any)
		totalNoise += noiseThisOctave * currentAmplitude;
		totalAmplitude += currentAmplitude;
		currentAmplitude *= octavePersistence;
		currentPos *= octaveScale;
		currentPos.x += OCTAVE_OFFSET; // Add "irrational" offsets to noise position components
		currentPos.y += OCTAVE_OFFSET; //	at each octave to break up their grid alignment
		++ seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if( renormalize && totalAmplitude > 0.f )
	{
		totalNoise /= totalAmplitude;				// Amplitude exceeds 1.0 if octaves are used
		totalNoise = (totalNoise * 0.5f) + 0.5f;	// Map to [0,1]
		totalNoise = SmoothStep3( totalNoise );		// Push towards extents (octaves pull us away)
		totalNoise = (totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
	}

	return totalNoise;
}


//-----------------------------------------------------------------------------------------------
float Compute3dFractalNoise( float posX, float posY, float posZ, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed )
{
	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave

	float totalNoise = 0.f;
	float totalAmplitude = 0.f;
	float currentAmplitude = 1.f;
	float invScale = (1.f / scale);
	Vector3 currentPos( posX * invScale, posY * invScale, posZ * invScale );

	for( unsigned int octaveNum = 0; octaveNum < numOctaves; ++ octaveNum )
	{
		// Determine noise values at nearby integer "grid point" positions
		Vector3 cellMins( floorf( currentPos.x ), floorf( currentPos.y ), floorf( currentPos.z ) );
		int indexWestX  = (int) cellMins.x;
		int indexSouthY = (int) cellMins.y;
		int indexBelowZ = (int) cellMins.z;
		int indexEastX  = indexWestX + 1;
		int indexNorthY = indexSouthY + 1;
		int indexAboveZ = indexBelowZ + 1;

		// Noise grid cell has 8 corners in 3D
		float aboveSouthWest = Get3dNoiseZeroToOne( indexWestX, indexSouthY, indexAboveZ, seed );
		float aboveSouthEast = Get3dNoiseZeroToOne( indexEastX, indexSouthY, indexAboveZ, seed );
		float aboveNorthWest = Get3dNoiseZeroToOne( indexWestX, indexNorthY, indexAboveZ, seed );
		float aboveNorthEast = Get3dNoiseZeroToOne( indexEastX, indexNorthY, indexAboveZ, seed );
		float belowSouthWest = Get3dNoiseZeroToOne( indexWestX, indexSouthY, indexBelowZ, seed );
		float belowSouthEast = Get3dNoiseZeroToOne( indexEastX, indexSouthY, indexBelowZ, seed );
		float belowNorthWest = Get3dNoiseZeroToOne( indexWestX, indexNorthY, indexBelowZ, seed );
		float belowNorthEast = Get3dNoiseZeroToOne( indexEastX, indexNorthY, indexBelowZ, seed );

		// Do a smoothed (nonlinear) weighted average of nearby grid point values
		Vector3 displacementFromMins = currentPos - cellMins;

		float weightEast  = SmoothStep3( displacementFromMins.x );
		float weightNorth = SmoothStep3( displacementFromMins.y );
		float weightAbove = SmoothStep3( displacementFromMins.z );
		float weightWest  = 1.f - weightEast;
		float weightSouth = 1.f - weightNorth;
		float weightBelow = 1.f - weightAbove;

		// 8-way blend (8 -> 4 -> 2 -> 1)
		float blendBelowSouth = (weightEast * belowSouthEast) + (weightWest * belowSouthWest);
		float blendBelowNorth = (weightEast * belowNorthEast) + (weightWest * belowNorthWest);
		float blendAboveSouth = (weightEast * aboveSouthEast) + (weightWest * aboveSouthWest);
		float blendAboveNorth = (weightEast * aboveNorthEast) + (weightWest * aboveNorthWest);
		float blendBelow = (weightSouth * blendBelowSouth) + (weightNorth * blendBelowNorth);
		float blendAbove = (weightSouth * blendAboveSouth) + (weightNorth * blendAboveNorth);
		float blendTotal = (weightBelow * blendBelow) + (weightAbove * blendAbove);
		float noiseThisOctave = 2.f * (blendTotal - 0.5f); // Map from [0,1] to [-1,1]

		// Accumulate results and prepare for next octave (if any)
		totalNoise += noiseThisOctave * currentAmplitude;
		totalAmplitude += currentAmplitude;
		currentAmplitude *= octavePersistence;
		currentPos *= octaveScale;
		currentPos.x += OCTAVE_OFFSET; // Add "irrational" offsets to noise position components
		currentPos.y += OCTAVE_OFFSET; //	at each octave to break up their grid alignment
		currentPos.z += OCTAVE_OFFSET;
		++ seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if( renormalize && totalAmplitude > 0.f )
	{
		totalNoise /= totalAmplitude;				// Amplitude exceeds 1.0 if octaves are used
		totalNoise = (totalNoise * 0.5f) + 0.5f;	// Map to [0,1]
		totalNoise = SmoothStep3( totalNoise );		// Push towards extents (octaves pull us away)
		totalNoise = (totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
	}

	return totalNoise;
}


//-----------------------------------------------------------------------------------------------
float Compute4dFractalNoise( float posX, float posY, float posZ, float posT, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed )
{
	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave

	float totalNoise = 0.f;
	float totalAmplitude = 0.f;
	float currentAmplitude = 1.f;
	float invScale = (1.f / scale);
	Vector4 currentPos( posX * invScale, posY * invScale, posZ * invScale, posT * invScale );

	for( unsigned int octaveNum = 0; octaveNum < numOctaves; ++ octaveNum )
	{
		// Determine noise values at nearby integer "grid point" positions
		Vector4 cellMins( floorf( currentPos.x ), floorf( currentPos.y ), floorf( currentPos.z ), floorf( currentPos.w ) );
		int indexWestX   = (int) cellMins.x;
		int indexSouthY  = (int) cellMins.y;
		int indexBelowZ  = (int) cellMins.z;
		int indexBeforeT = (int) cellMins.w;
		int indexEastX  = indexWestX + 1;
		int indexNorthY = indexSouthY + 1;
		int indexAboveZ = indexBelowZ + 1;
		int indexAfterT = indexBeforeT + 1;

		// Noise grid cell has 16 "corners" in 4D
		float beforeBelowSW = Get4dNoiseZeroToOne( indexWestX, indexSouthY, indexBelowZ, indexBeforeT, seed );
		float beforeBelowSE = Get4dNoiseZeroToOne( indexEastX, indexSouthY, indexBelowZ, indexBeforeT, seed );
		float beforeBelowNW = Get4dNoiseZeroToOne( indexWestX, indexNorthY, indexBelowZ, indexBeforeT, seed );
		float beforeBelowNE = Get4dNoiseZeroToOne( indexEastX, indexNorthY, indexBelowZ, indexBeforeT, seed );
		float beforeAboveSW = Get4dNoiseZeroToOne( indexWestX, indexSouthY, indexAboveZ, indexBeforeT, seed );
		float beforeAboveSE = Get4dNoiseZeroToOne( indexEastX, indexSouthY, indexAboveZ, indexBeforeT, seed );
		float beforeAboveNW = Get4dNoiseZeroToOne( indexWestX, indexNorthY, indexAboveZ, indexBeforeT, seed );
		float beforeAboveNE = Get4dNoiseZeroToOne( indexEastX, indexNorthY, indexAboveZ, indexBeforeT, seed );

		float afterBelowSW = Get4dNoiseZeroToOne( indexWestX, indexSouthY, indexBelowZ, indexAfterT, seed );
		float afterBelowSE = Get4dNoiseZeroToOne( indexEastX, indexSouthY, indexBelowZ, indexAfterT, seed );
		float afterBelowNW = Get4dNoiseZeroToOne( indexWestX, indexNorthY, indexBelowZ, indexAfterT, seed );
		float afterBelowNE = Get4dNoiseZeroToOne( indexEastX, indexNorthY, indexBelowZ, indexAfterT, seed );
		float afterAboveSW = Get4dNoiseZeroToOne( indexWestX, indexSouthY, indexAboveZ, indexAfterT, seed );
		float afterAboveSE = Get4dNoiseZeroToOne( indexEastX, indexSouthY, indexAboveZ, indexAfterT, seed );
		float afterAboveNW = Get4dNoiseZeroToOne( indexWestX, indexNorthY, indexAboveZ, indexAfterT, seed );
		float afterAboveNE = Get4dNoiseZeroToOne( indexEastX, indexNorthY, indexAboveZ, indexAfterT, seed );

		// Do a smoothed (nonlinear) weighted average of nearby grid point values
		Vector4 displacementFromMins = currentPos - cellMins;

		float weightEast	= SmoothStep3( displacementFromMins.x );
		float weightNorth	= SmoothStep3( displacementFromMins.y );
		float weightAbove	= SmoothStep3( displacementFromMins.z );
		float weightAfter	= SmoothStep3( displacementFromMins.w );
		float weightWest	= 1.f - weightEast;
		float weightSouth	= 1.f - weightNorth;
		float weightBelow	= 1.f - weightAbove;
		float weightBefore	= 1.f - weightAfter;

		// 16-way blend (16 -> 8 -> 4 -> 2 -> 1)
		float blendBeforeBelowSouth	= (weightEast * beforeBelowSE) + (weightWest * beforeBelowSW);
		float blendBeforeBelowNorth	= (weightEast * beforeBelowNE) + (weightWest * beforeBelowNW);
		float blendBeforeAboveSouth	= (weightEast * beforeAboveSE) + (weightWest * beforeAboveSW);
		float blendBeforeAboveNorth	= (weightEast * beforeAboveNE) + (weightWest * beforeAboveNW);
		float blendAfterBelowSouth  = (weightEast * afterBelowSE)  + (weightWest * afterBelowSW);
		float blendAfterBelowNorth  = (weightEast * afterBelowNE)  + (weightWest * afterBelowNW);
		float blendAfterAboveSouth  = (weightEast * afterAboveSE)  + (weightWest * afterAboveSW);
		float blendAfterAboveNorth  = (weightEast * afterAboveNE)  + (weightWest * afterAboveNW);
		float blendBeforeBelow = (weightSouth * blendBeforeBelowSouth) + (weightNorth * blendBeforeBelowNorth);
		float blendBeforeAbove = (weightSouth * blendBeforeAboveSouth) + (weightNorth * blendBeforeAboveNorth);
		float blendAfterBelow  = (weightSouth * blendAfterBelowSouth)  + (weightNorth * blendAfterBelowNorth);
		float blendAfterAbove  = (weightSouth * blendAfterAboveSouth)  + (weightNorth * blendAfterAboveNorth);
		float blendBefore = (weightBelow * blendBeforeBelow) + (weightAbove * blendBeforeAbove);
		float blendAfter  = (weightBelow * blendAfterBelow) + (weightAbove * blendAfterAbove);
		float blendTotal = (weightBefore * blendBefore) + (weightAfter * blendAfter);
		float noiseThisOctave = 2.f * (blendTotal - 0.5f); // Map from [0,1] to [-1,1]

		// Accumulate results and prepare for next octave (if any)
		totalNoise += noiseThisOctave * currentAmplitude;
		totalAmplitude += currentAmplitude;
		currentAmplitude *= octavePersistence;
		currentPos *= octaveScale;
		currentPos.x += OCTAVE_OFFSET; // Add "irrational" offsets to noise position components
		currentPos.y += OCTAVE_OFFSET; //	at each octave to break up their grid alignment
		currentPos.z += OCTAVE_OFFSET;
		currentPos.w += OCTAVE_OFFSET;
		++ seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if( renormalize && totalAmplitude > 0.f )
	{
		totalNoise /= totalAmplitude;				// Amplitude exceeds 1.0 if octaves are used
		totalNoise = (totalNoise * 0.5f) + 0.5f;	// Map to [0,1]
		totalNoise = SmoothStep3( totalNoise );		// Push towards extents (octaves pull us away)
		totalNoise = (totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
	}

	return totalNoise;
}


//-----------------------------------------------------------------------------------------------
// Perlin noise is fractal noise with "gradient vector smoothing" applied.
//
// In 1D, the gradients are trivial: -1.0 or 1.0, so resulting noise is boring at one octave.
//
float Compute1dPerlinNoise( float position, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed )
{
	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave
	const float gradients[2] = { -1.f, 1.f }; // 1D unit "gradient" vectors; one back, one forward

	float totalNoise = 0.f;
	float totalAmplitude = 0.f;
	float currentAmplitude = 1.f;
	float currentPosition = position * (1.f / scale);

	for( unsigned int octaveNum = 0; octaveNum < numOctaves; ++ octaveNum )
	{
		// Determine random "gradient vectors" (just +1 or -1 for 1D Perlin) for surrounding corners
		float positionFloor = (float) floorf( currentPosition );
		int indexWest = (int) positionFloor;
		int indexEast = indexWest + 1;
		float gradientWest = gradients[ Get1dNoiseUint( indexWest, seed ) & 0x00000001 ];
		float gradientEast = gradients[ Get1dNoiseUint( indexEast, seed ) & 0x00000001 ];

		// Dot each point's gradient with displacement from point to position
		float displacementFromWest = currentPosition - positionFloor; // always positive
		float displacementFromEast = displacementFromWest - 1.f; // always negative
		float dotWest = gradientWest * displacementFromWest; // 1D "dot product" is... multiply
		float dotEast = gradientEast * displacementFromEast;

		// Do a smoothed (nonlinear) weighted average of dot results
		float weightEast = SmoothStep3( displacementFromWest );
		float weightWest = 1.f - weightEast;
		float blendTotal = (weightWest * dotWest) + (weightEast * dotEast);
		float noiseThisOctave = 2.f * blendTotal; // 1D Perlin is in [-.5,.5]; map to [-1,1]

		// Accumulate results and prepare for next octave (if any)
		totalNoise += noiseThisOctave * currentAmplitude;
		totalAmplitude += currentAmplitude;
		currentAmplitude *= octavePersistence;
		currentPosition *= octaveScale;
		currentPosition += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		++ seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if( renormalize && totalAmplitude > 0.f )
	{
		totalNoise /= totalAmplitude;				// Amplitude exceeds 1.0 if octaves are used
		totalNoise = (totalNoise * 0.5f) + 0.5f;	// Map to [0,1]
		totalNoise = SmoothStep3( totalNoise );		// Push towards extents (octaves pull us away)
		totalNoise = (totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
	}

	return totalNoise;
}

float Compute1dPerlinNoiseZeroToOne(float position, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed)
{
	float noise = Compute1dPerlinNoise(position, scale, numOctaves, octavePersistence, octaveScale, renormalize, seed);
	noise = (noise * 0.5f) + 0.5f;
	return noise;
}

//-----------------------------------------------------------------------------------------------
// Perlin noise is fractal noise with "gradient vector smoothing" applied.
//
// In 2D, gradients are unit-length vectors in various directions with even angular distribution.
//
float Compute2dPerlinNoise( float posX, float posY, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed )
{
	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave
	const Vector2 gradients[ 8 ] = // Normalized unit vectors in 8 quarter-cardinal directions
	{
		Vector2( +0.923879533f, +0.382683432f ), //  22.5 degrees (ENE)
		Vector2( +0.382683432f, +0.923879533f ), //  67.5 degrees (NNE)
		Vector2( -0.382683432f, +0.923879533f ), // 112.5 degrees (NNW)
		Vector2( -0.923879533f, +0.382683432f ), // 157.5 degrees (WNW)
		Vector2( -0.923879533f, -0.382683432f ), // 202.5 degrees (WSW)
		Vector2( -0.382683432f, -0.923879533f ), // 247.5 degrees (SSW)
		Vector2( +0.382683432f, -0.923879533f ), // 292.5 degrees (SSE)
		Vector2( +0.923879533f, -0.382683432f )	 // 337.5 degrees (ESE)
	};

	float totalNoise = 0.f;
	float totalAmplitude = 0.f;
	float currentAmplitude = 1.f;
	float invScale = (1.f / scale);
	Vector2 currentPos( posX * invScale, posY * invScale );

	for( unsigned int octaveNum = 0; octaveNum < numOctaves; ++ octaveNum )
	{
		// Determine random unit "gradient vectors" for surrounding corners
		Vector2 cellMins( floorf( currentPos.x ), floorf( currentPos.y ) );
		Vector2 cellMaxs( cellMins.x + 1.f, cellMins.y + 1.f );
		int indexWestX  = (int) cellMins.x;
		int indexSouthY = (int) cellMins.y;
		int indexEastX  = indexWestX  + 1;
		int indexNorthY = indexSouthY + 1;

		unsigned int noiseSW = Get2dNoiseUint( indexWestX, indexSouthY, seed );
		unsigned int noiseSE = Get2dNoiseUint( indexEastX, indexSouthY, seed );
		unsigned int noiseNW = Get2dNoiseUint( indexWestX, indexNorthY, seed );
		unsigned int noiseNE = Get2dNoiseUint( indexEastX, indexNorthY, seed );

		const Vector2& gradientSW = gradients[ noiseSW & 0x00000007 ];
		const Vector2& gradientSE = gradients[ noiseSE & 0x00000007 ];
		const Vector2& gradientNW = gradients[ noiseNW & 0x00000007 ];
		const Vector2& gradientNE = gradients[ noiseNE & 0x00000007 ];

		// Dot each corner's gradient with displacement from corner to position
		Vector2 displacementFromSW( currentPos.x - cellMins.x, currentPos.y - cellMins.y );
		Vector2 displacementFromSE( currentPos.x - cellMaxs.x, currentPos.y - cellMins.y );
		Vector2 displacementFromNW( currentPos.x - cellMins.x, currentPos.y - cellMaxs.y );
		Vector2 displacementFromNE( currentPos.x - cellMaxs.x, currentPos.y - cellMaxs.y );

		float dotSouthWest = DotProduct( gradientSW, displacementFromSW );
		float dotSouthEast = DotProduct( gradientSE, displacementFromSE );
		float dotNorthWest = DotProduct( gradientNW, displacementFromNW );
		float dotNorthEast = DotProduct( gradientNE, displacementFromNE );

		// Do a smoothed (nonlinear) weighted average of dot results
		float weightEast = SmoothStep3( displacementFromSW.x );
		float weightNorth = SmoothStep3( displacementFromSW.y );
		float weightWest = 1.f - weightEast;
		float weightSouth = 1.f - weightNorth;

		float blendSouth = (weightEast * dotSouthEast) + (weightWest * dotSouthWest);
		float blendNorth = (weightEast * dotNorthEast) + (weightWest * dotNorthWest);
		float blendTotal = (weightSouth * blendSouth) + (weightNorth * blendNorth);
		float noiseThisOctave = 1.5f * blendTotal; // 2D Perlin is in ~[-.66,.66]; map to ~[-1,1]
		
		// Accumulate results and prepare for next octave (if any)
		totalNoise += noiseThisOctave * currentAmplitude;
		totalAmplitude += currentAmplitude;
		currentAmplitude *= octavePersistence;
		currentPos *= octaveScale;
		currentPos.x += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.y += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		++ seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if( renormalize && totalAmplitude > 0.f )
	{
		totalNoise /= totalAmplitude;				// Amplitude exceeds 1.0 if octaves are used
		totalNoise = (totalNoise * 0.5f) + 0.5f;	// Map to [0,1]
		totalNoise = SmoothStep3( totalNoise );		// Push towards extents (octaves pull us away)
		totalNoise = (totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
	}

	return totalNoise;
}

float Compute2dPerlinNoiseZeroToOne(float posX, float posY, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed)
{
	float noise = Compute2dPerlinNoise(posX, posY, scale, numOctaves, octavePersistence, octaveScale, renormalize, seed);
	noise = (noise * 0.5f) + 0.5f;
	return noise;
}

//-----------------------------------------------------------------------------------------------
// Perlin noise is fractal noise with "gradient vector smoothing" applied.
//
// In 3D, gradients are unit-length vectors in random (3D) directions.
//
float Compute3dPerlinNoise( float posX, float posY, float posZ, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed )
{
	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave
	const float fSQRT_3_OVER_3 = sqrtf(3.0f) / 3.0f;

	const Vector3 gradients[ 8 ] = // Traditional "12 edges" requires modulus and isn't any better.
	{
		Vector3( +fSQRT_3_OVER_3, +fSQRT_3_OVER_3, +fSQRT_3_OVER_3 ), // Normalized unit 3D vectors
		Vector3( -fSQRT_3_OVER_3, +fSQRT_3_OVER_3, +fSQRT_3_OVER_3 ), //  pointing toward cube
		Vector3( +fSQRT_3_OVER_3, -fSQRT_3_OVER_3, +fSQRT_3_OVER_3 ), //  corners, so components
		Vector3( -fSQRT_3_OVER_3, -fSQRT_3_OVER_3, +fSQRT_3_OVER_3 ), //  are all sqrt(3)/3, i.e.
		Vector3( +fSQRT_3_OVER_3, +fSQRT_3_OVER_3, -fSQRT_3_OVER_3 ), // 0.5773502691896257645091f.
		Vector3( -fSQRT_3_OVER_3, +fSQRT_3_OVER_3, -fSQRT_3_OVER_3 ), // These are slightly better
		Vector3( +fSQRT_3_OVER_3, -fSQRT_3_OVER_3, -fSQRT_3_OVER_3 ), // than axes (1,0,0) and much
		Vector3( -fSQRT_3_OVER_3, -fSQRT_3_OVER_3, -fSQRT_3_OVER_3 )  // faster than edges (1,1,0).
	};

	float totalNoise = 0.f;
	float totalAmplitude = 0.f;
	float currentAmplitude = 1.f;
	float invScale = (1.f / scale);
	Vector3 currentPos( posX * invScale, posY * invScale, posZ * invScale );

	for( unsigned int octaveNum = 0; octaveNum < numOctaves; ++ octaveNum )
	{
		// Determine random unit "gradient vectors" for surrounding corners
		Vector3 cellMins( floorf( currentPos.x ), floorf( currentPos.y ), floorf( currentPos.z ) );
		Vector3 cellMaxs( cellMins.x + 1.f, cellMins.y + 1.f, cellMins.z + 1.f );
		int indexWestX  = (int) cellMins.x;
		int indexSouthY = (int) cellMins.y;
		int indexBelowZ = (int) cellMins.z;
		int indexEastX  = indexWestX  + 1;
		int indexNorthY = indexSouthY + 1;
		int indexAboveZ = indexBelowZ + 1;

		unsigned int noiseBelowSW = Get3dNoiseUint( indexWestX, indexSouthY, indexBelowZ, seed );
		unsigned int noiseBelowSE = Get3dNoiseUint( indexEastX, indexSouthY, indexBelowZ, seed );
		unsigned int noiseBelowNW = Get3dNoiseUint( indexWestX, indexNorthY, indexBelowZ, seed );
		unsigned int noiseBelowNE = Get3dNoiseUint( indexEastX, indexNorthY, indexBelowZ, seed );
		unsigned int noiseAboveSW = Get3dNoiseUint( indexWestX, indexSouthY, indexAboveZ, seed );
		unsigned int noiseAboveSE = Get3dNoiseUint( indexEastX, indexSouthY, indexAboveZ, seed );
		unsigned int noiseAboveNW = Get3dNoiseUint( indexWestX, indexNorthY, indexAboveZ, seed );
		unsigned int noiseAboveNE = Get3dNoiseUint( indexEastX, indexNorthY, indexAboveZ, seed );

		Vector3 gradientBelowSW = gradients[ noiseBelowSW & 0x00000007 ];
		Vector3 gradientBelowSE = gradients[ noiseBelowSE & 0x00000007 ];
		Vector3 gradientBelowNW = gradients[ noiseBelowNW & 0x00000007 ];
		Vector3 gradientBelowNE = gradients[ noiseBelowNE & 0x00000007 ];
		Vector3 gradientAboveSW = gradients[ noiseAboveSW & 0x00000007 ];
		Vector3 gradientAboveSE = gradients[ noiseAboveSE & 0x00000007 ];
		Vector3 gradientAboveNW = gradients[ noiseAboveNW & 0x00000007 ];
		Vector3 gradientAboveNE = gradients[ noiseAboveNE & 0x00000007 ];

		// Dot each corner's gradient with displacement from corner to position
		Vector3 displacementFromBelowSW( currentPos.x - cellMins.x, currentPos.y - cellMins.y, currentPos.z - cellMins.z );
		Vector3 displacementFromBelowSE( currentPos.x - cellMaxs.x, currentPos.y - cellMins.y, currentPos.z - cellMins.z );
		Vector3 displacementFromBelowNW( currentPos.x - cellMins.x, currentPos.y - cellMaxs.y, currentPos.z - cellMins.z );
		Vector3 displacementFromBelowNE( currentPos.x - cellMaxs.x, currentPos.y - cellMaxs.y, currentPos.z - cellMins.z );
		Vector3 displacementFromAboveSW( currentPos.x - cellMins.x, currentPos.y - cellMins.y, currentPos.z - cellMaxs.z );
		Vector3 displacementFromAboveSE( currentPos.x - cellMaxs.x, currentPos.y - cellMins.y, currentPos.z - cellMaxs.z );
		Vector3 displacementFromAboveNW( currentPos.x - cellMins.x, currentPos.y - cellMaxs.y, currentPos.z - cellMaxs.z );
		Vector3 displacementFromAboveNE( currentPos.x - cellMaxs.x, currentPos.y - cellMaxs.y, currentPos.z - cellMaxs.z );

		float dotBelowSW = DotProduct( gradientBelowSW, displacementFromBelowSW );
		float dotBelowSE = DotProduct( gradientBelowSE, displacementFromBelowSE );
		float dotBelowNW = DotProduct( gradientBelowNW, displacementFromBelowNW );
		float dotBelowNE = DotProduct( gradientBelowNE, displacementFromBelowNE );
		float dotAboveSW = DotProduct( gradientAboveSW, displacementFromAboveSW );
		float dotAboveSE = DotProduct( gradientAboveSE, displacementFromAboveSE );
		float dotAboveNW = DotProduct( gradientAboveNW, displacementFromAboveNW );
		float dotAboveNE = DotProduct( gradientAboveNE, displacementFromAboveNE );

		// Do a smoothed (nonlinear) weighted average of dot results
		float weightEast  = SmoothStep3( displacementFromBelowSW.x );
		float weightNorth = SmoothStep3( displacementFromBelowSW.y );
		float weightAbove = SmoothStep3( displacementFromBelowSW.z );
		float weightWest  = 1.f - weightEast;
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
		float noiseThisOctave = 1.66666666f * blendTotal; // 3D Perlin is ~[-.6,.6]; map to ~[-1,1]

		// Accumulate results and prepare for next octave (if any)
		totalNoise += noiseThisOctave * currentAmplitude;
		totalAmplitude += currentAmplitude;
		currentAmplitude *= octavePersistence;
		currentPos *= octaveScale;
		currentPos.x += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.y += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.z += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		++ seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if( renormalize && totalAmplitude > 0.f )
	{
		totalNoise /= totalAmplitude;				// Amplitude exceeds 1.0 if octaves are used
		totalNoise = (totalNoise * 0.5f) + 0.5f;	// Map to [0,1]
		totalNoise = SmoothStep3( totalNoise );		// Push towards extents (octaves pull us away)
		totalNoise = (totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
	}

	return totalNoise;
}

float Compute3dPerlinNoiseZeroToOne(float posX, float posY, float posZ, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed)
{
	float noise = Compute3dPerlinNoise(posX, posY, posZ, scale, numOctaves, octavePersistence, octaveScale, renormalize, seed);
	noise = (noise * 0.5f) + 0.5f;
	return noise;
}

//-----------------------------------------------------------------------------------------------
// Perlin noise is fractal noise with "gradient vector smoothing" applied.
//
// In 4D, gradients are unit-length hyper-vectors in random (4D) directions.
//
float Compute4dPerlinNoise( float posX, float posY, float posZ, float posT, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed )
{
	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave

	const Vector4 gradients[ 16 ] = // Hard to tell if this is any better in 4D than just having 8
	{
		Vector4( +0.5f, +0.5f, +0.5f, +0.5f ), // Normalized unit 4D vectors pointing toward each
		Vector4( -0.5f, +0.5f, +0.5f, +0.5f ), //  of the 16 hypercube corners, so components are
		Vector4( +0.5f, -0.5f, +0.5f, +0.5f ), //  all sqrt(4)/4, i.e. one-half.
		Vector4( -0.5f, -0.5f, +0.5f, +0.5f ), // 
		Vector4( +0.5f, +0.5f, -0.5f, +0.5f ), // It's hard to tell whether these are any better
		Vector4( -0.5f, +0.5f, -0.5f, +0.5f ), //  or worse than vectors facing axes (1,0,0,0) or
		Vector4( +0.5f, -0.5f, -0.5f, +0.5f ), //  3D edges (.7,.7,0,0) or 4D edges (.57,.57,.57,0)
		Vector4( -0.5f, -0.5f, -0.5f, +0.5f ), //  but less-axial gradients looked a little better
		Vector4( +0.5f, +0.5f, +0.5f, -0.5f ), //  with 2D and 3D noise so I'm assuming this is as
		Vector4( -0.5f, +0.5f, +0.5f, -0.5f ), //  good or better as any other gradient-selection
		Vector4( +0.5f, -0.5f, +0.5f, -0.5f ), //  scheme (and is crazy-fast).  *shrug*
		Vector4( -0.5f, -0.5f, +0.5f, -0.5f ), // 
		Vector4( +0.5f, +0.5f, -0.5f, -0.5f ), // Plus, we want a power-of-two number of evenly-
		Vector4( -0.5f, +0.5f, -0.5f, -0.5f ), //  distributed gradients, so we can cheaply select
		Vector4( +0.5f, -0.5f, -0.5f, -0.5f ), //  one from bit-noise (use bit-mask, not modulus).
		Vector4( -0.5f, -0.5f, -0.5f, -0.5f )  // 
	};

	float totalNoise = 0.f;
	float totalAmplitude = 0.f;
	float currentAmplitude = 1.f;
	float invScale = (1.f / scale);
	Vector4 currentPos( posX * invScale, posY * invScale, posZ * invScale, posT * invScale );

	for( unsigned int octaveNum = 0; octaveNum < numOctaves; ++ octaveNum )
	{
		// Determine random unit "gradient vectors" for 16 surrounding 4D (hypercube) cell corners
		Vector4 cellMins( floorf( currentPos.x ), floorf( currentPos.y ), floorf( currentPos.z ), floorf( currentPos.w ) );
		Vector4 cellMaxs( cellMins.x + 1.f, cellMins.y + 1.f, cellMins.z + 1.f, cellMins.w + 1.f );
		int indexWestX   = (int) cellMins.x;
		int indexSouthY  = (int) cellMins.y;
		int indexBelowZ  = (int) cellMins.z;
		int indexBeforeT = (int) cellMins.w;
		int indexEastX  = indexWestX   + 1;
		int indexNorthY = indexSouthY  + 1;
		int indexAboveZ = indexBelowZ  + 1;
		int indexAfterT = indexBeforeT + 1;

		// "BeforeBSW" stands for "BeforeBelowSouthWest" below (i.e. 4D hypercube mins), etc.
		unsigned int noiseBeforeBSW = Get4dNoiseUint( indexWestX, indexSouthY, indexBelowZ, indexBeforeT, seed );
		unsigned int noiseBeforeBSE = Get4dNoiseUint( indexEastX, indexSouthY, indexBelowZ, indexBeforeT, seed );
		unsigned int noiseBeforeBNW = Get4dNoiseUint( indexWestX, indexNorthY, indexBelowZ, indexBeforeT, seed );
		unsigned int noiseBeforeBNE = Get4dNoiseUint( indexEastX, indexNorthY, indexBelowZ, indexBeforeT, seed );
		unsigned int noiseBeforeASW = Get4dNoiseUint( indexWestX, indexSouthY, indexAboveZ, indexBeforeT, seed );
		unsigned int noiseBeforeASE = Get4dNoiseUint( indexEastX, indexSouthY, indexAboveZ, indexBeforeT, seed );
		unsigned int noiseBeforeANW = Get4dNoiseUint( indexWestX, indexNorthY, indexAboveZ, indexBeforeT, seed );
		unsigned int noiseBeforeANE = Get4dNoiseUint( indexEastX, indexNorthY, indexAboveZ, indexBeforeT, seed );
		unsigned int noiseAfterBSW = Get4dNoiseUint( indexWestX, indexSouthY, indexBelowZ, indexAfterT, seed );
		unsigned int noiseAfterBSE = Get4dNoiseUint( indexEastX, indexSouthY, indexBelowZ, indexAfterT, seed );
		unsigned int noiseAfterBNW = Get4dNoiseUint( indexWestX, indexNorthY, indexBelowZ, indexAfterT, seed );
		unsigned int noiseAfterBNE = Get4dNoiseUint( indexEastX, indexNorthY, indexBelowZ, indexAfterT, seed );
		unsigned int noiseAfterASW = Get4dNoiseUint( indexWestX, indexSouthY, indexAboveZ, indexAfterT, seed );
		unsigned int noiseAfterASE = Get4dNoiseUint( indexEastX, indexSouthY, indexAboveZ, indexAfterT, seed );
		unsigned int noiseAfterANW = Get4dNoiseUint( indexWestX, indexNorthY, indexAboveZ, indexAfterT, seed );
		unsigned int noiseAfterANE = Get4dNoiseUint( indexEastX, indexNorthY, indexAboveZ, indexAfterT, seed );

		// Mask with 15 (mod 16) to look up in gradients table
		Vector4 gradientBeforeBSW = gradients[ noiseBeforeBSW & 0x0000000F ];
		Vector4 gradientBeforeBSE = gradients[ noiseBeforeBSE & 0x0000000F ];
		Vector4 gradientBeforeBNW = gradients[ noiseBeforeBNW & 0x0000000F ];
		Vector4 gradientBeforeBNE = gradients[ noiseBeforeBNE & 0x0000000F ];
		Vector4 gradientBeforeASW = gradients[ noiseBeforeASW & 0x0000000F ];
		Vector4 gradientBeforeASE = gradients[ noiseBeforeASE & 0x0000000F ];
		Vector4 gradientBeforeANW = gradients[ noiseBeforeANW & 0x0000000F ];
		Vector4 gradientBeforeANE = gradients[ noiseBeforeANE & 0x0000000F ];
		Vector4 gradientAfterBSW = gradients[ noiseAfterBSW & 0x0000000F ];
		Vector4 gradientAfterBSE = gradients[ noiseAfterBSE & 0x0000000F ];
		Vector4 gradientAfterBNW = gradients[ noiseAfterBNW & 0x0000000F ];
		Vector4 gradientAfterBNE = gradients[ noiseAfterBNE & 0x0000000F ];
		Vector4 gradientAfterASW = gradients[ noiseAfterASW & 0x0000000F ];
		Vector4 gradientAfterASE = gradients[ noiseAfterASE & 0x0000000F ];
		Vector4 gradientAfterANW = gradients[ noiseAfterANW & 0x0000000F ];
		Vector4 gradientAfterANE = gradients[ noiseAfterANE & 0x0000000F ];

		// Dot each corner's gradient with displacement from corner to position
		Vector4 displacementFromBeforeBSW( currentPos.x - cellMins.x, currentPos.y - cellMins.y, currentPos.z - cellMins.z, currentPos.w - cellMins.w );
		Vector4 displacementFromBeforeBSE( currentPos.x - cellMaxs.x, currentPos.y - cellMins.y, currentPos.z - cellMins.z, currentPos.w - cellMins.w );
		Vector4 displacementFromBeforeBNW( currentPos.x - cellMins.x, currentPos.y - cellMaxs.y, currentPos.z - cellMins.z, currentPos.w - cellMins.w );
		Vector4 displacementFromBeforeBNE( currentPos.x - cellMaxs.x, currentPos.y - cellMaxs.y, currentPos.z - cellMins.z, currentPos.w - cellMins.w );
		Vector4 displacementFromBeforeASW( currentPos.x - cellMins.x, currentPos.y - cellMins.y, currentPos.z - cellMaxs.z, currentPos.w - cellMins.w );
		Vector4 displacementFromBeforeASE( currentPos.x - cellMaxs.x, currentPos.y - cellMins.y, currentPos.z - cellMaxs.z, currentPos.w - cellMins.w );
		Vector4 displacementFromBeforeANW( currentPos.x - cellMins.x, currentPos.y - cellMaxs.y, currentPos.z - cellMaxs.z, currentPos.w - cellMins.w );
		Vector4 displacementFromBeforeANE( currentPos.x - cellMaxs.x, currentPos.y - cellMaxs.y, currentPos.z - cellMaxs.z, currentPos.w - cellMins.w );
		Vector4 displacementFromAfterBSW( currentPos.x - cellMins.x, currentPos.y - cellMins.y, currentPos.z - cellMins.z, currentPos.w - cellMaxs.w );
		Vector4 displacementFromAfterBSE( currentPos.x - cellMaxs.x, currentPos.y - cellMins.y, currentPos.z - cellMins.z, currentPos.w - cellMaxs.w );
		Vector4 displacementFromAfterBNW( currentPos.x - cellMins.x, currentPos.y - cellMaxs.y, currentPos.z - cellMins.z, currentPos.w - cellMaxs.w );
		Vector4 displacementFromAfterBNE( currentPos.x - cellMaxs.x, currentPos.y - cellMaxs.y, currentPos.z - cellMins.z, currentPos.w - cellMaxs.w );
		Vector4 displacementFromAfterASW( currentPos.x - cellMins.x, currentPos.y - cellMins.y, currentPos.z - cellMaxs.z, currentPos.w - cellMaxs.w );
		Vector4 displacementFromAfterASE( currentPos.x - cellMaxs.x, currentPos.y - cellMins.y, currentPos.z - cellMaxs.z, currentPos.w - cellMaxs.w );
		Vector4 displacementFromAfterANW( currentPos.x - cellMins.x, currentPos.y - cellMaxs.y, currentPos.z - cellMaxs.z, currentPos.w - cellMaxs.w );
		Vector4 displacementFromAfterANE( currentPos.x - cellMaxs.x, currentPos.y - cellMaxs.y, currentPos.z - cellMaxs.z, currentPos.w - cellMaxs.w );

		float dotBeforeBSW = DotProduct( gradientBeforeBSW, displacementFromBeforeBSW );
		float dotBeforeBSE = DotProduct( gradientBeforeBSE, displacementFromBeforeBSE );
		float dotBeforeBNW = DotProduct( gradientBeforeBNW, displacementFromBeforeBNW );
		float dotBeforeBNE = DotProduct( gradientBeforeBNE, displacementFromBeforeBNE );
		float dotBeforeASW = DotProduct( gradientBeforeASW, displacementFromBeforeASW );
		float dotBeforeASE = DotProduct( gradientBeforeASE, displacementFromBeforeASE );
		float dotBeforeANW = DotProduct( gradientBeforeANW, displacementFromBeforeANW );
		float dotBeforeANE = DotProduct( gradientBeforeANE, displacementFromBeforeANE );
		float dotAfterBSW = DotProduct( gradientAfterBSW, displacementFromAfterBSW );
		float dotAfterBSE = DotProduct( gradientAfterBSE, displacementFromAfterBSE );
		float dotAfterBNW = DotProduct( gradientAfterBNW, displacementFromAfterBNW );
		float dotAfterBNE = DotProduct( gradientAfterBNE, displacementFromAfterBNE );
		float dotAfterASW = DotProduct( gradientAfterASW, displacementFromAfterASW );
		float dotAfterASE = DotProduct( gradientAfterASE, displacementFromAfterASE );
		float dotAfterANW = DotProduct( gradientAfterANW, displacementFromAfterANW );
		float dotAfterANE = DotProduct( gradientAfterANE, displacementFromAfterANE );

		// Do a smoothed (nonlinear) weighted average of dot results
		float weightEast  = SmoothStep3( displacementFromBeforeBSW.x );
		float weightNorth = SmoothStep3( displacementFromBeforeBSW.y );
		float weightAbove = SmoothStep3( displacementFromBeforeBSW.z );
		float weightAfter = SmoothStep3( displacementFromBeforeBSW.w );
		float weightWest   = 1.f - weightEast;
		float weightSouth  = 1.f - weightNorth;
		float weightBelow  = 1.f - weightAbove;
		float weightBefore = 1.f - weightAfter;

		// 16-way blend (16 -> 8 -> 4 -> 2 -> 1)
		float blendBeforeBelowSouth	= (weightEast * dotBeforeBSE) + (weightWest * dotBeforeBSW);
		float blendBeforeBelowNorth	= (weightEast * dotBeforeBNE) + (weightWest * dotBeforeBNW);
		float blendBeforeAboveSouth	= (weightEast * dotBeforeASE) + (weightWest * dotBeforeASW);
		float blendBeforeAboveNorth	= (weightEast * dotBeforeANE) + (weightWest * dotBeforeANW);
		float blendAfterBelowSouth  = (weightEast * dotAfterBSE)  + (weightWest * dotAfterBSW);
		float blendAfterBelowNorth  = (weightEast * dotAfterBNE)  + (weightWest * dotAfterBNW);
		float blendAfterAboveSouth  = (weightEast * dotAfterASE)  + (weightWest * dotAfterASW);
		float blendAfterAboveNorth  = (weightEast * dotAfterANE)  + (weightWest * dotAfterANW);
		float blendBeforeBelow = (weightSouth * blendBeforeBelowSouth) + (weightNorth * blendBeforeBelowNorth);
		float blendBeforeAbove = (weightSouth * blendBeforeAboveSouth) + (weightNorth * blendBeforeAboveNorth);
		float blendAfterBelow  = (weightSouth * blendAfterBelowSouth)  + (weightNorth * blendAfterBelowNorth);
		float blendAfterAbove  = (weightSouth * blendAfterAboveSouth)  + (weightNorth * blendAfterAboveNorth);
		float blendBefore = (weightBelow * blendBeforeBelow) + (weightAbove * blendBeforeAbove);
		float blendAfter  = (weightBelow * blendAfterBelow) + (weightAbove * blendAfterAbove);
		float blendTotal = (weightBefore * blendBefore) + (weightAfter * blendAfter);
		float noiseThisOctave = 1.6f * blendTotal; // 4D Perlin is in ~[-.5,.5]; map to ~[-1,1]

		// Accumulate results and prepare for next octave (if any)
		totalNoise += noiseThisOctave * currentAmplitude;
		totalAmplitude += currentAmplitude;
		currentAmplitude *= octavePersistence;
		currentPos *= octaveScale;
		currentPos.x += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.y += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.z += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.w += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		++ seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if( renormalize && totalAmplitude > 0.f )
	{
		totalNoise /= totalAmplitude;				// Amplitude exceeds 1.0 if octaves are used
		totalNoise = (totalNoise * 0.5f) + 0.5f;	// Map to [0,1]
		totalNoise = SmoothStep3( totalNoise );		// Push towards extents (octaves pull us away)
		totalNoise = (totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
	}

	return totalNoise;
}

float Compute4dPerlinNoiseZeroToOne(float posX, float posY, float posZ, float posT, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed)
{
	float noise = Compute4dPerlinNoise(posX, posY, posZ, posT, scale, numOctaves, octavePersistence, octaveScale, renormalize, seed);
	noise = (noise * 0.5f) + 0.5f;
	return noise;
}

float get_worley_noise_2d(float x, float y)
{
    return get_worley_noise_2d(Vector2(x, y));
}

//-----------------------------------------------------------------------------------------------
float get_worley_noise_2d(const Vector2& in_pos)
{
    float x_int;
    float x_frac = modf(in_pos.x, &x_int);

    float y_int;
    float y_frac = modf(in_pos.y, &y_int);

    Vector2 i_st(x_int, y_int);
    Vector2 f_st(x_frac, y_frac);

    float m_dist = 1.0;  // minimun distance
    for(int y = -1; y <= 1; y++) {
        for(int x = -1; x <= 1; x++) {
            // Neighbor place in the grid
            Vector2 neighbor = Vector2((float)x, (float)y);

            // Random position from current + neighbor place in the grid
            Vector2 p = random2(i_st + neighbor);

            // Vector between the pixel and the point
            Vector2 diff = neighbor + p - f_st;

            // Distance to the point
            float dist = diff.CalcLength();

            // Keep the closer distance
            m_dist = Min(m_dist, dist);
        }
    }

    return m_dist;
}

//-----------------------------------------------------------------------------------------------
float get_worley_noise_2d(Vector2 position, float scale, unsigned int num_octaves, float octave_persistence, float octave_scale, bool renormalize)
{
    float total_noise = 0.f;
    float total_amplitude = 0.f;
    float current_amplitude = 1.f;
    Vector2 current_position = position * scale;

    for(unsigned int octave = 0; octave < num_octaves; ++octave)
    {
        float noise_this_octave = get_worley_noise_2d(current_position);

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

float get_worley_noise_2d(float x, float y, float scale, unsigned int num_octaves, float octave_persistence, float octave_scale, bool renormalize)
{
    return get_worley_noise_2d(Vector2(x, y), scale ,num_octaves, octave_persistence, octave_scale, renormalize);
}

//-----------------------------------------------------------------------------------------------
float get_worley_noise_3d(float x_pos, float y_pos, float z_pos)
{
    return get_worley_noise_3d(Vector3(x_pos, y_pos, z_pos));
}

float get_worley_noise_3d(Vector3 position)
{
    float x_int;
    float x_frac = modf(position.x, &x_int);

    float y_int;
    float y_frac = modf(position.y, &y_int);

    float z_int;
    float z_frac = modf(position.z, &z_int);

    Vector3 i_st(x_int, y_int, z_int);
    Vector3 f_st(x_frac, y_frac, z_frac);

    float m_dist = 1.0;  // minimun distance
    for(int z = -1; z <= 1; z++){
        for(int y = -1; y <= 1; y++) {
            for(int x = -1; x <= 1; x++) {
                // Neighbor place in the grid
                Vector3 neighbor = Vector3((float)x, (float)y, (float)z);

                // Random position from current + neighbor place in the grid
                Vector3 p = random3(i_st + neighbor);

                // Vector between the pixel and the point
                Vector3 diff = neighbor + p - f_st;

                // Distance to the point
                float dist = diff.CalcLength();

                // Keep the closer distance
                m_dist = Min(m_dist, dist);
            }
        }
    }

    return m_dist;
}

float get_worley_noise_3d(Vector3 position, float scale, unsigned int num_octaves, float octave_persistence, float octave_scale, bool renormalize)
{
    float total_noise = 0.f;
    float total_amplitude = 0.f;
    float current_amplitude = 1.f;
    Vector3 current_position = position * scale;

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
    return get_worley_noise_3d(Vector3(x, y, z), scale, num_octaves, octave_persistence, octave_scale, renormalize);
}