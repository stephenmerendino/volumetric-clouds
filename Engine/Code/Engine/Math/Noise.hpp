//-----------------------------------------------------------------------------------------------
// Noise.hpp
//
#pragma once

#include "Engine/Math/EasingFuncs.hpp"
#include "Engine/Core/random.h"


/////////////////////////////////////////////////////////////////////////////////////////////////
// Squirrel's Noise utilities (version 3)
//
// This code is made available under the Creative Commons attribution 3.0 license (CC-BY-3.0 US):
//	Attribution in source code comments (even closed-source/commercial code) is sufficient.
//	License summary and text available at: https://creativecommons.org/licenses/by/3.0/us/
//
// These noise functions were written by Squirrel Eiserloh as a cheap and simple substitute for
//	the [frankly awful] bit-noise & fractal noise sample code functions commonly found on the web,
//	many of which are hugely biased or terribly patterned, e.g. having bits which are on (or off)
//	75% or even 100% of the time.
//
// Note: This is work in progress, and has not yet been tested thoroughly.  Use at your own risk.
//	Please report any bugs, issues, or bothersome cases to SquirrelEiserloh at gmail.com.
//
// The following functions are all based on a simple bit-noise function which returns an unsigned
//	integer containing 32 reasonably-well-scrambled bits, based on a given (signed) integer
//	input parameter (position/index) and [optional] seed.  Kind of like looking up a value in an
//	infinitely large [non-existent] table of previously rolled random numbers.
//
// These functions are deterministic and random-access / order-independent (i.e. state-free),
//	so they are particularly well-suited for use in smoothed/fractal/simplex/Perlin noise
//	functions and out-of-order (or or-demand) procedural content generation (i.e. that mountain
//	village is the same whether you generated it first or last, ahead of time or just now).
//
// The N-dimensional variations simply hash their multidimensional coordinates down to a single
//	32-bit index and then proceed as usual, so while results are not unique they should
//	(hopefully) not seem locally predictable or repetitive.
//
// Note: these functions assume the presence of a few simple math utility functions and classes;
//	class Vector2, Vector3, Vector4								// public x,y,z,w floats, operators
//	float DotProduct( const Vector2& a, const Vector2& b )		// (ax*bx) + (ay*by)
//	float DotProduct( const Vector3& a, const Vector3& b )		// (ax*bx) + (ay*by) + (az*bz)
//	float DotProduct( const Vector4& a, const Vector4& b )		// (ax*bx) + (ay*by) + (az*bz) + (aw*bw)
//	float SmoothStep( float inputZeroToOne )					// 3t^2 - 2t^3		(^ is power-of)
//
/////////////////////////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------------------------
// Raw pseudorandom noise functions (random-access / deterministic).  Basis of all other noise.
//
unsigned int Get1dNoiseUint( int index, unsigned int seed=0 );
unsigned int Get2dNoiseUint( int indexX, int indexY, unsigned int seed=0 );
unsigned int Get3dNoiseUint( int indexX, int indexY, int indexZ, unsigned int seed=0 );
unsigned int Get4dNoiseUint( int indexX, int indexY, int indexZ, int indexT, unsigned int seed=0 );

//-----------------------------------------------------------------------------------------------
// Same functions, mapped to floats in [0,1] for convenience.
//
float Get1dNoiseZeroToOne( int index, unsigned int seed=0 );
float Get2dNoiseZeroToOne( int indexX, int indexY, unsigned int seed=0 );
float Get3dNoiseZeroToOne( int indexX, int indexY, int indexZ, unsigned int seed=0 );
float Get4dNoiseZeroToOne( int indexX, int indexY, int indexZ, int indexT, unsigned int seed=0 );

//-----------------------------------------------------------------------------------------------
// Same functions, mapped to floats in [-1,1] for convenience.
//
float Get1dNoiseNegOneToOne( int index, unsigned int seed=0 );
float Get2dNoiseNegOneToOne( int indexX, int indexY, unsigned int seed=0 );
float Get3dNoiseNegOneToOne( int indexX, int indexY, int indexZ, unsigned int seed=0 );
float Get4dNoiseNegOneToOne( int indexX, int indexY, int indexZ, int indexT, unsigned int seed=0 );


//-----------------------------------------------------------------------------------------------
// Smooth/fractal pseudorandom noise functions (random-access / deterministic)
//
// These are less "organic" (and more axial) than Perlin's functions, but simpler and faster.
// 
// <numOctaves>			Number of layers of noise added together
// <octavePersistence>	Amplitude multiplier for each subsequent octave (each octave is quieter)
// <octaveScale>		Frequency multiplier for each subsequent octave (each octave is busier)
// <renormalize>		If true, uses nonlinear (SmoothStep) renormalization to within [-1,1]
//
float Compute1dFractalNoise( float position, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );
float Compute2dFractalNoise( float posX, float posY, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );
float Compute3dFractalNoise( float posX, float posY, float posZ, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );
float Compute4dFractalNoise( float posX, float posY, float posZ, float posT, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );


//-----------------------------------------------------------------------------------------------
// Perlin noise functions (random-access / deterministic)
//
// Perlin noise is slightly more expensive, but more organic-looking (less axial) than regular
//	square fractal noise, through the use of blended dot products vs. randomized gradient vectors.
//
// <numOctaves>			Number of layers of noise added together
// <octavePersistence>	Amplitude multiplier for each subsequent octave (each octave is quieter)
// <octaveScale>		Frequency multiplier for each subsequent octave (each octave is busier)
// <renormalize>		If true, uses nonlinear (SmoothStep) renormalization to within [-1,1]
//
float Compute1dPerlinNoise( float position, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );
float Compute2dPerlinNoise( float posX, float posY, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );
float Compute3dPerlinNoise( float posX, float posY, float posZ, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );
float Compute4dPerlinNoise( float posX, float posY, float posZ, float posT, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );

float Compute1dPerlinNoiseZeroToOne( float position, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );
float Compute2dPerlinNoiseZeroToOne( float posX, float posY, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );
float Compute3dPerlinNoiseZeroToOne( float posX, float posY, float posZ, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );
float Compute4dPerlinNoiseZeroToOne( float posX, float posY, float posZ, float posT, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );

//-----------------------------------------------------------------------------------------------
// Simplex noise functions (random-access / deterministic)
//
// Simplex noise (also by Ken Perlin) is supposedly faster than - and apparently superior to - 
//	Perlin noise, in that it is more organic-looking.  I'm not sure I like the look of it better,
//	though, and examples of cross-sectional 4D simplex noise look worse to me than 4D Perlin.
//
// Simplex noise is based on a regular simplex (2D triangle, 3D tetrahedron, 4-simplex/5-cell)
//	grid, which is a bit more fiddly, so I haven't written my own yet.
//
// #TODO: Implement simplex noise in 2D, 3D, 4D (1D simplex is identical to 1D Perlin, I think?)
// #TODO: Test actual simplex noise implementation in 2D/3D to compare speeds (branches vs. math!)


//-----------------------------------------------------------------------------------------------
inline unsigned int Get2dNoiseUint( int indexX, int indexY, unsigned int seed )
{
	const int PRIME_NUMBER = 198491317; // Large prime number with non-boring bits
	return Get1dNoiseUint( indexX + (PRIME_NUMBER * indexY), seed );
}


//-----------------------------------------------------------------------------------------------
inline unsigned int Get3dNoiseUint( int indexX, int indexY, int indexZ, unsigned int seed )
{
	const int PRIME1 = 198491317; // Large prime number with non-boring bits
	const int PRIME2 = 6542989; // Large prime number with distinct and non-boring bits
	return Get1dNoiseUint( indexX + (PRIME1 * indexY) + (PRIME2 * indexZ), seed );
}


//-----------------------------------------------------------------------------------------------
inline unsigned int Get4dNoiseUint( int indexX, int indexY, int indexZ, int indexT, unsigned int seed )
{
	const int PRIME1 = 198491317; // Large prime number with non-boring bits
	const int PRIME2 = 6542989; // Large prime number with distinct and non-boring bits
	const int PRIME3 = 357239; // Large prime number with distinct and non-boring bits
	return Get1dNoiseUint( indexX + (PRIME1 * indexY) + (PRIME2 * indexZ) + (PRIME3 * indexT), seed );
}


//-----------------------------------------------------------------------------------------------
inline float Get1dNoiseZeroToOne( int index, unsigned int seed )
{
	const double ONE_OVER_MAX_UINT = (1.0 / (double) 0xFFFFFFFF);
	return (float)( ONE_OVER_MAX_UINT * (double) Get1dNoiseUint( index, seed ) );
}


//-----------------------------------------------------------------------------------------------
inline float Get2dNoiseZeroToOne( int indexX, int indexY, unsigned int seed )
{
	const double ONE_OVER_MAX_UINT = (1.0 / (double) 0xFFFFFFFF);
	return (float)( ONE_OVER_MAX_UINT * (double) Get2dNoiseUint( indexX, indexY, seed ) );
}


//-----------------------------------------------------------------------------------------------
inline float Get3dNoiseZeroToOne( int indexX, int indexY, int indexZ, unsigned int seed )
{
	const double ONE_OVER_MAX_UINT = (1.0 / (double) 0xFFFFFFFF);
	return (float)( ONE_OVER_MAX_UINT * (double) Get3dNoiseUint( indexX, indexY, indexZ, seed ) );
}


//-----------------------------------------------------------------------------------------------
inline float Get4dNoiseZeroToOne( int indexX, int indexY, int indexZ, int indexT, unsigned int seed )
{
	const double ONE_OVER_MAX_UINT = (1.0 / (double) 0xFFFFFFFF);
	return (float)( ONE_OVER_MAX_UINT * (double) Get4dNoiseUint( indexX, indexY, indexZ, indexT, seed ) );
}


//-----------------------------------------------------------------------------------------------
inline float Get1dNoiseNegOneToOne( int index, unsigned int seed )
{
	const double ONE_OVER_MAX_INT = (1.0 / (double) 0x7FFFFFFF);
	return (float)( ONE_OVER_MAX_INT * (double) (int) Get1dNoiseUint( index, seed ) );
}


//-----------------------------------------------------------------------------------------------
inline float Get2dNoiseNegOneToOne( int indexX, int indexY, unsigned int seed )
{
	const double ONE_OVER_MAX_INT = (1.0 / (double) 0x7FFFFFFF);
	return (float)( ONE_OVER_MAX_INT * (double) (int) Get2dNoiseUint( indexX, indexY, seed ) );
}


//-----------------------------------------------------------------------------------------------
inline float Get3dNoiseNegOneToOne( int indexX, int indexY, int indexZ, unsigned int seed )
{
	const double ONE_OVER_MAX_INT = (1.0 / (double) 0x7FFFFFFF);
	return (float)( ONE_OVER_MAX_INT * (double) (int) Get3dNoiseUint( indexX, indexY, indexZ, seed ) );
}


//-----------------------------------------------------------------------------------------------
inline float Get4dNoiseNegOneToOne( int indexX, int indexY, int indexZ, int indexT, unsigned int seed )
{
	const double ONE_OVER_MAX_INT = (1.0 / (double) 0x7FFFFFFF);
	return (float)( ONE_OVER_MAX_INT * (double) (int) Get4dNoiseUint( indexX, indexY, indexZ, indexT, seed ) );
}

//-----------------------------------------------------------------------------------------------
float get_worley_noise_2d(const Vector2& in_pos);
float get_worley_noise_2d(float x, float y);

//-----------------------------------------------------------------------------------------------
float get_worley_noise_2d(Vector2 position, float scale, unsigned int num_octaves, float octave_persistence, float octave_scale, bool renormalize);
float get_worley_noise_2d(float x, float y, float scale, unsigned int num_octaves, float octave_persistence, float octave_scale, bool renormalize);

//-----------------------------------------------------------------------------------------------
float get_worley_noise_3d(Vector3 position);
float get_worley_noise_3d(float x, float y, float z);

//-----------------------------------------------------------------------------------------------
float get_worley_noise_3d(Vector3 position, float scale, unsigned int num_octaves, float octave_persistence, float octave_scale, bool renormalize);
float get_worley_noise_3d(float x, float y, float z, float scale, unsigned int num_octaves, float octave_persistence, float octave_scale, bool renormalize);