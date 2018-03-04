#pragma once

#include "Engine/Renderer/SpriteSheet.hpp"

enum SpriteAnimMode{
	SPRITE_ANIM_MODE_PLAY_TO_END,
	SPRITE_ANIM_MODE_LOOPING,
	NUM_SPRITE_ANIM_MODES
};

class SpriteAnim{
public:
	SpriteAnim(SpriteSheet& spriteSheet, float durationSeconds, 
					SpriteAnimMode playbackMode, int startSpriteIndex, int endSpriteIndex);

	void Update(float deltaSeconds);
	AABB2 GetCurrentTexCoords() const;
	int GetNumFrames() const;
	RHITexture2D& GetTexture() const;
	void Pause();
	void Resume();
	void Reset();

	bool IsFinished()				const	{ return m_isFinished; }
	bool IsPlaying()				const	{ return m_isPlaying; }
	float GetDurationSeconds()		const	{ return m_durationSeconds; }
	float GetSecondsElapsed()		const	{ return m_elapsedSeconds; }

	float GetSecondsRemaining() const;
	float GetFractionElapsed() const;
	float GetFractionRemaining() const;
	void SetSecondsElapsed(float secondsElapsed);
	void SetFractionElapsed(float fractionElapsed);

private:
	SpriteSheet& m_spriteSheet;
	int m_startSpriteIndex;
	int m_endSpriteIndex;
	
	SpriteAnimMode m_playbackMode;

	bool m_isFinished;
	bool m_isPlaying;

	float m_durationSeconds;
	float m_elapsedSeconds;
};