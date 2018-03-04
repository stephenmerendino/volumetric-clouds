#pragma once

class RHIDevice;
class RHIDeviceContext;
struct ID3D11Buffer;

class ConstantBuffer
{
public:
	ID3D11Buffer* m_dxBuffer;
	size_t m_bufferSize;

public:
	ConstantBuffer(RHIDevice* owner, const void* buffer, const size_t bufferSize);
	~ConstantBuffer();

	inline bool IsValid() const { return (m_dxBuffer != nullptr); }

	bool Update(RHIDeviceContext* context, const void* buffer);
};