#pragma once

#include "Engine/RHI/RHITypes.hpp"

class RHIDevice;
class RHIDeviceContext;
struct ID3D11Buffer;

class IndexBuffer
{
public:
	unsigned int		m_numIndexes;
	unsigned int		m_capacity;
	BufferUsage			m_bufferUsage;
	ID3D11Buffer*		m_dxBuffer;

public:
	IndexBuffer(RHIDevice* device, const unsigned int* indexes, const unsigned int numIndexes, const BufferUsage bufferUsage = BUFFERUSAGE_STATIC);
	~IndexBuffer();

	inline bool IsValid() const { return m_dxBuffer != nullptr; }
	inline unsigned int GetCount() const { return m_numIndexes; }

	void Create(RHIDevice* device, const unsigned int* indexes, const unsigned int numIndexes);
	bool Update(RHIDeviceContext* context, const unsigned int* indexes, const unsigned int numIndexes);
};