#pragma once

#include <string>
#include <vector>
#include <map>

enum PlaybackMode{
	PLAYBACK_MODE_PLAY_TO_END,
	PLAYBACK_MODE_LOOP,
	NUM_MODES
};

#if !defined( TOOLS_BUILD )
//---------------------------------------------------------------------------
#pragma comment( lib, "ThirdParty/fmod/fmodex_vc" ) // Link in the fmodex_vc.lib static library

//---------------------------------------------------------------------------
#include "ThirdParty/fmod/fmod.hpp"


//---------------------------------------------------------------------------
typedef unsigned int SoundID;
typedef void* AudioChannelHandle;
const unsigned int MISSING_SOUND_ID = 0xffffffff;


/////////////////////////////////////////////////////////////////////////////
class AudioSystem
{
public:
	AudioSystem();
	virtual ~AudioSystem();

	SoundID CreateOrGetSound( const std::string& soundFileName );
	AudioChannelHandle PlaySoundByID( SoundID soundID, float volumeLevel=1.f, PlaybackMode playbackMode = PLAYBACK_MODE_PLAY_TO_END );
	void StopChannel( AudioChannelHandle channel );
	void Update();

protected:
	void InitializeFMOD();
	void ShutDownFMOD();
	void ValidateResult( FMOD_RESULT result );

protected:
	FMOD::System*						m_fmodSystem;
	std::map< std::string, SoundID >	m_registeredSoundIDs;
	std::vector< FMOD::Sound* >			m_registeredSounds;
};

#else

typedef int SoundID;
typedef int AudioChannelHandle;

class AudioSystem
{
public:
	AudioSystem(){};
	virtual ~AudioSystem(){};

	SoundID CreateOrGetSound(const std::string& soundFileName);
	AudioChannelHandle PlaySoundByID(SoundID soundID, float volumeLevel = 1.f, PlaybackMode playbackMode = PLAYBACK_MODE_PLAY_TO_END);
	void StopChannel(AudioChannelHandle channel);
	void Update();
};

#endif