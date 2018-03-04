#pragma once

class SimpleRenderer;

class Renderable
{
public:
	virtual ~Renderable(){};
	virtual void draw() = 0;
};