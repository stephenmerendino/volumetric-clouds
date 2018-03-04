#include "Engine/Math/MatrixStack.hpp"

MatrixStack::MatrixStack()
{
	clear();
}

MatrixStack::~MatrixStack()
{
}

void MatrixStack::clear()
{
	m_stack = std::stack<Matrix4>();
	m_top = Matrix4::IDENTITY;
}

void MatrixStack::push(const Matrix4& matrix)
{
	m_stack.push(m_top);
	m_top = m_top * matrix;
}

void MatrixStack::push_direct(const Matrix4& matrix)
{
	m_stack.push(m_top);
	m_top = matrix;
}

void MatrixStack::pop()
{
	m_stack.pop();
}

Matrix4 MatrixStack::top()
{
	return m_top;
}