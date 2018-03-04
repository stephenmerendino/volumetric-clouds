#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Camera3d.hpp"
#include "Engine/Core/Common.hpp"

#include "Engine/Math/Noise.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Game::Game()
	:m_camera(nullptr)
{
	m_camera = new Camera3d(Vector3::ZERO);
}

Game::~Game()
{
}

void Game::Update(float deltaSeconds)
{
	m_camera->Update(deltaSeconds);
}

void Game::Render()
{
}