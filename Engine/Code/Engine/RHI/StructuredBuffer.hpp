#pragma once

class RHIDevice;
class RHIDeviceContext;
struct ID3D11Buffer;
struct ID3D11ShaderResourceView;

class StructuredBuffer
{
	public:
		RHIDevice* m_device;

		ID3D11Buffer* m_dx_buffer;
		ID3D11ShaderResourceView* m_dx_srv;

		unsigned int m_obj_count;
		unsigned int m_obj_size;
		size_t m_buffer_size;

	public:
		StructuredBuffer(RHIDevice *owner, 
						 const void* initial_data, 
						 unsigned int object_size, 
						 unsigned int object_count);
		~StructuredBuffer();

		bool create(const void* initial_data);
		bool create_views();
		bool update(RHIDeviceContext *context, const void* buffer);
		bool is_valid() const;
};