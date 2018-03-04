#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/VertexBuffer.hpp"
#include "Engine/RHI/RHITypes.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/Renderer/Vertex3.hpp"

VertexBuffer::VertexBuffer(RHIDevice* device, const Vertex3* vertexes, const unsigned int numVertexes, BufferUsage bufferUsage)
	:m_dxBuffer(nullptr)
	,m_numVertexes(numVertexes)
	,m_capacity(numVertexes)
	,m_bufferUsage(bufferUsage)
{
	Create(device, vertexes, numVertexes);
}

VertexBuffer::~VertexBuffer()
{
	DX_SAFE_RELEASE(m_dxBuffer);
}

void VertexBuffer::Create(RHIDevice* device, const Vertex3* vertexes, const unsigned int numVertexes)
{
	// Release the old buffer
	DX_SAFE_RELEASE(m_dxBuffer);

	m_numVertexes = numVertexes;
	m_capacity = numVertexes;

	// First, describe the buffer
	D3D11_BUFFER_DESC vb_desc;
	memset(&vb_desc, 0, sizeof(vb_desc));

	vb_desc.ByteWidth = sizeof(Vertex3) * numVertexes;  // How much data are we putting into this buffer
	vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;         // What can we bind this data as (in this case, only vertex data)
	vb_desc.Usage = DXGetBufferUsage(m_bufferUsage);                // Hint on how this memory is used (in this case, it is immutable, or constant - can't be changed)
														  // for limitations/strenghts of each, see;  
														  //    https://msdn.microsoft.com/en-us/library/windows/desktop/ff476259(v=vs.85).aspx
	vb_desc.StructureByteStride = sizeof(Vertex3);       // How large is each element in this buffer

	D3D11_SUBRESOURCE_DATA initial_data;
	memset(&initial_data, 0, sizeof(initial_data));
	initial_data.pSysMem = vertexes;

	if (m_bufferUsage == BUFFERUSAGE_DYNAMIC){
		// Finally create the vertex buffer
		vb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	// Next, setup the initial data (required since this is an immutable buffer - so it must be instantiated at creation time)
	if (vertexes != nullptr){
		// Finally create the vertex buffer
		device->m_dxDevice->CreateBuffer(&vb_desc, &initial_data, &m_dxBuffer);
	}
	else{
		device->m_dxDevice->CreateBuffer(&vb_desc, NULL, &m_dxBuffer);
	}
}

bool VertexBuffer::VertexBuffer::Update(RHIDeviceContext* context, const Vertex3* vertexes, const unsigned int numVertexes)
{
	if (numVertexes > m_capacity){
		// Create a new one with the correct size
		Create(context->m_device, vertexes, numVertexes);
		if(!IsValid()){
			return false;
		}
	}

	m_numVertexes = numVertexes;

	// Update the already existing vbo
	D3D11_MAPPED_SUBRESOURCE resource;
	
	ID3D11DeviceContext* dxContext = context->m_dxDeviceContext;
	if (SUCCEEDED(dxContext->Map(m_dxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0U, &resource))){
		size_t bufferSize = m_numVertexes * sizeof(Vertex3);
		memcpy(resource.pData, vertexes, bufferSize);
		dxContext->Unmap(m_dxBuffer, 0);
		return true;
	}

	return false;
}