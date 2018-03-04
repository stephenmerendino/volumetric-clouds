#pragma once

#include "Engine/RHI/RHITypes.hpp"

class RHIDevice;
class RHIDeviceContext;
class Vertex3;
struct ID3D11Buffer;

class VertexBuffer
{
public:
	unsigned int		m_numVertexes;
	unsigned int		m_capacity;
	BufferUsage			m_bufferUsage;
	ID3D11Buffer*		m_dxBuffer;

public:
	VertexBuffer(RHIDevice* device, const Vertex3* vertexes, const unsigned int numVertexes, BufferUsage bufferUsage = BUFFERUSAGE_STATIC);
	~VertexBuffer(); 

	inline bool IsValid() const { return (m_dxBuffer != nullptr); }

	bool Update(RHIDeviceContext* context, const Vertex3* vertexes, const unsigned int numVertexes);

private:
	void Create(RHIDevice* device, const Vertex3* vertexes, const unsigned int numVertexes);
};