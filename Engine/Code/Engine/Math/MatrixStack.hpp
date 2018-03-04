#pragma once

#include "Engine/Math/Matrix4.hpp"
#include <stack>

class MatrixStack
{
public:
	Matrix4 m_top;
	std::stack<Matrix4> m_stack;

public:
	MatrixStack();
	~MatrixStack();

	void clear();
	void push(const Matrix4& matrix);
	void push_direct(const Matrix4& matrix);
	void pop();
	Matrix4 top();
};