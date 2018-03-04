#include "Engine/RHI/IndexBuffer.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHITypes.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"
#include "Engine/RHI/DX11.hpp"

IndexBuffer::IndexBuffer(RHIDevice* device, const unsigned int* indexes, const unsigned int numIndexes, const BufferUsage bufferUsage)
	:m_dxBuffer(nullptr)
	,m_numIndexes(numIndexes)
	,m_capacity(numIndexes)
	,m_bufferUsage(bufferUsage)
{
	Create(device, indexes, numIndexes);
}

IndexBuffer::~IndexBuffer()
{
	DX_SAFE_RELEASE(m_dxBuffer);
}

void IndexBuffer::Create(RHIDevice* device, const unsigned int* indexes, const unsigned int numIndexes)
{
	// Release the old buffer if it exists
	DX_SAFE_RELEASE(m_dxBuffer);

	m_numIndexes = numIndexes;
	m_capacity = m_numIndexes;

	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));

	bufferDesc.Usage = DXGetBufferUsage(m_bufferUsage);
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(unsigned int) * numIndexes;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = sizeof(unsigned int);

	if (m_bufferUsage == BUFFERUSAGE_DYNAMIC){
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	if (indexes != nullptr){
		D3D11_SUBRESOURCE_DATA initialData;
		initialData.pSysMem = indexes;
		initialData.SysMemPitch = 0;
		initialData.SysMemSlicePitch = 0;

		device->m_dxDevice->CreateBuffer(&bufferDesc, &initialData, &m_dxBuffer);
	}
	else{
		device->m_dxDevice->CreateBuffer(&bufferDesc, NULL, &m_dxBuffer);
	}
}

bool IndexBuffer::Update(RHIDeviceContext* context, const unsigned int* indexes, const unsigned int numIndexes)
{
	if (numIndexes > m_capacity){
		Create(context->m_device, indexes, numIndexes);
		if(!IsValid()){
			return false;
		}
	}

	m_numIndexes = numIndexes;

	D3D11_MAPPED_SUBRESOURCE resource;
	memset(&resource, 0, sizeof(resource));
	
	ID3D11DeviceContext* dxContext = context->m_dxDeviceContext;
	if (SUCCEEDED(dxContext->Map(m_dxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0U, &resource))){
		size_t bufferSize = m_numIndexes * sizeof(unsigned int);
		memcpy(resource.pData, indexes, bufferSize);
		dxContext->Unmap(m_dxBuffer, 0);
		return true;
	}

	return false;
}