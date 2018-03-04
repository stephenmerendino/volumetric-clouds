#include "Engine/Renderer/SpriteAnim.hpp"

SpriteAnim::SpriteAnim(SpriteSheet& spriteSheet, float durationSeconds, 
					   SpriteAnimMode playbackMode, int startSpriteIndex, int endSpriteIndex)
	:m_spriteSheet(spriteSheet),
	 m_durationSeconds(durationSeconds),
	 m_playbackMode(playbackMode),
	 m_startSpriteIndex(startSpriteIndex),
	 m_endSpriteIndex(endSpriteIndex),
	 m_isPlaying(true),
	 m_isFinished(false),
	 m_elapsedSeconds(0)
{
}

void SpriteAnim::Update(float deltaSeconds){
	if(m_isPlaying){
		m_elapsedSeconds += deltaSeconds;
		if(m_playbackMode == SPRITE_ANIM_MODE_PLAY_TO_END){
			if(m_elapsedSeconds >= m_durationSeconds){
				m_isFinished = true;
				m_isPlaying = false;
			}
		} else if(m_playbackMode == SPRITE_ANIM_MODE_LOOPING){
			if(m_elapsedSeconds >= m_durationSeconds){
				while(m_elapsedSeconds >= m_durationSeconds)
					m_elapsedSeconds -= m_durationSeconds;
			}
		} 
	}
}

AABB2 SpriteAnim::GetCurrentTexCoords() const{
	float fractionElapsed = GetFractionElapsed();
	int elapsedFrames = (int)(GetNumFrames() * fractionElapsed);

	int currentSpriteIndex = m_startSpriteIndex + elapsedFrames;

	return m_spriteSheet.GetTexCoordsForSpriteIndex(currentSpriteIndex);
}

int SpriteAnim::GetNumFrames() const{
	return m_endSpriteIndex - m_startSpriteIndex;
}

RHITexture2D& SpriteAnim::GetTexture() const{
	return m_spriteSheet.GetTexture();
}

void SpriteAnim::Pause(){
	m_isPlaying = false;
}

void SpriteAnim::Resume(){
	m_isPlaying = true;
}

void SpriteAnim::Reset(){
	m_elapsedSeconds = 0;
}

float SpriteAnim::GetSecondsRemaining() const{
	return m_durationSeconds - m_elapsedSeconds;
}

float SpriteAnim::GetFractionElapsed() const{
	return m_elapsedSeconds / m_durationSeconds;
}

float SpriteAnim::GetFractionRemaining() const{
	return GetSecondsRemaining() / m_durationSeconds;
}

void SpriteAnim::SetSecondsElapsed(float secondsElapsed){
	m_elapsedSeconds = secondsElapsed;
}

void SpriteAnim::SetFractionElapsed(float fractionElapsed){
	m_elapsedSeconds = m_durationSeconds * fractionElapsed;
}