#include "Engine/RHI/StructuredBuffer.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Common.hpp"

StructuredBuffer::StructuredBuffer(RHIDevice *owner,
								   const void* initial_data,
								   unsigned int object_size,
								   unsigned int object_count)
	:m_device(owner)
	,m_dx_srv(nullptr)
	,m_dx_buffer(nullptr)
	,m_buffer_size(object_size * object_count)
	,m_obj_count(object_count)
	,m_obj_size(object_size)
{
	create(initial_data);
}

StructuredBuffer::~StructuredBuffer()
{
	DX_SAFE_RELEASE(m_dx_buffer);
	DX_SAFE_RELEASE(m_dx_srv);
}

bool StructuredBuffer::create(const void* initial_data)
{
	DX_SAFE_RELEASE(m_dx_buffer);
	DX_SAFE_RELEASE(m_dx_srv);

   D3D11_BUFFER_DESC vb_desc;
   MemZero(&vb_desc);

   vb_desc.ByteWidth = (unsigned int)m_buffer_size;
   vb_desc.Usage = D3D11_USAGE_DYNAMIC;
   vb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
   vb_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
   vb_desc.StructureByteStride = m_obj_size;
   vb_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED; 

   D3D11_SUBRESOURCE_DATA dx_subresource_data;
   MemZero(&dx_subresource_data);
   dx_subresource_data.pSysMem = initial_data;
   
   m_device->m_dxDevice->CreateBuffer(&vb_desc, &dx_subresource_data, &m_dx_buffer);

   create_views();

   return true;
}

bool StructuredBuffer::create_views()
{
   D3D11_SHADER_RESOURCE_VIEW_DESC desc;
   MemZero(&desc);

   desc.Format = DXGI_FORMAT_UNKNOWN; 
   desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER; 
   desc.Buffer.ElementOffset = 0U;
   desc.Buffer.NumElements = m_obj_count;

   HRESULT result = m_device->m_dxDevice->CreateShaderResourceView(m_dx_buffer, &desc, &m_dx_srv); 
   ASSERT_OR_DIE(SUCCEEDED(result), "Failed to create shader resource view for structured buffer");
 
   return true;
}

bool StructuredBuffer::update(RHIDeviceContext *context, const void* buffer)
{
   D3D11_MAPPED_SUBRESOURCE resource;

   ID3D11DeviceContext *dx_context = context->m_dxDeviceContext;
   if (SUCCEEDED(dx_context->Map(m_dx_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0U, &resource ))) {
      memcpy(resource.pData, buffer, m_buffer_size);
      dx_context->Unmap(m_dx_buffer, 0);

      return true;
   }

   return false;
}

bool StructuredBuffer::is_valid() const 
{ 
	return m_dx_buffer != nullptr;
}