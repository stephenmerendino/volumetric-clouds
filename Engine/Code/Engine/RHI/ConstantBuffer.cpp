#include "Engine/RHI/ConstantBuffer.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"
#include "Engine/RHI/DX11.hpp"

ConstantBuffer::ConstantBuffer(RHIDevice* owner, const void* buffer, const size_t bufferSize)
	:m_bufferSize(bufferSize)
{
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.ByteWidth = bufferSize;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA initialData;
	memset(&initialData, 0, sizeof(initialData));
	initialData.pSysMem = buffer;

	owner->m_dxDevice->CreateBuffer(&bufferDesc, &initialData, &m_dxBuffer);
}

ConstantBuffer::~ConstantBuffer()
{
	DX_SAFE_RELEASE(m_dxBuffer);
}

bool ConstantBuffer::Update(RHIDeviceContext* context, const void* buffer)
{
	D3D11_MAPPED_SUBRESOURCE resource;

	ID3D11DeviceContext* dxContext = context->m_dxDeviceContext;

	if (SUCCEEDED(dxContext->Map(m_dxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0U, &resource))){
		memcpy(resource.pData, buffer, m_bufferSize);
		dxContext->Unmap(m_dxBuffer, 0);
		return true;
	}

	return false;
}
