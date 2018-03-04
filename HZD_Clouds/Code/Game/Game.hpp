#pragma once

class Camera3d;

class Game
{
public:
	Camera3d* m_camera;

public:
	Game();
	~Game();

public:
	void Update(float deltaSeconds);
	void Render();
};