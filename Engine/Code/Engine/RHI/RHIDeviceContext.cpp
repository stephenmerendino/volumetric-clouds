#include "Engine/RHI/RHIDeviceContext.hpp"
#include "Engine/RHI/RHITextureBase.hpp"
#include "Engine/RHI/RHITexture2D.hpp"
#include "Engine/RHI/RHITexture3D.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/RHI/ShaderProgram.hpp"
#include "Engine/RHI/VertexBuffer.hpp"
#include "Engine/RHI/IndexBuffer.hpp"
#include "Engine/RHI/RasterState.hpp"
#include "Engine/RHI/BlendState.hpp"
#include "Engine/RHI/DepthStencilState.hpp"
#include "Engine/RHI/ConstantBuffer.hpp"
#include "Engine/RHI/StructuredBuffer.hpp"
#include "Engine/RHI/ComputeJob.hpp"
#include "Engine/RHI/ComputeShader.hpp"
#include "Engine/RHI/ComputeShaderStage.hpp"
#include "Engine/Renderer/Vertex3.hpp"
#include "Engine/Profile/gpu_profile.h"
#include "Engine/Core/ErrorWarningAssert.hpp"

RHIDeviceContext::RHIDeviceContext(RHIDevice* owner, ID3D11DeviceContext* dx11DeviceContext)
	:m_device(owner)
	,m_dxDeviceContext(dx11DeviceContext)
{
}

RHIDeviceContext::~RHIDeviceContext()
{
	DX_SAFE_RELEASE(m_dxDeviceContext);
}

void RHIDeviceContext::ClearState()
{
	if(!m_dxDeviceContext){
		return;
	}

	m_dxDeviceContext->ClearState();
}

void RHIDeviceContext::Flush()
{
	if(!m_dxDeviceContext){
		return;
	}

	m_dxDeviceContext->Flush();
}

void RHIDeviceContext::SetColorTarget(RHITextureBase* colorTarget, RHITextureBase* depthStencilTarget)
{
	if(colorTarget->IsRenderTarget()){
		m_dxDeviceContext->OMSetRenderTargets(1, 
											  &colorTarget->m_dxRenderTargetView, 
											  depthStencilTarget != nullptr ? depthStencilTarget->m_dxDepthStencilView : nullptr);
	}
}

void RHIDeviceContext::SetColorTargets(RHITextureBase** colorTargets, int numColorTargets, RHITextureBase* depthStencilTarget)
{
	ID3D11RenderTargetView** dxRenderTargets = new ID3D11RenderTargetView*[numColorTargets];

	for(int targetIndex = 0; targetIndex < numColorTargets; ++targetIndex){
		dxRenderTargets[targetIndex] = colorTargets[targetIndex] != nullptr ? colorTargets[targetIndex]->m_dxRenderTargetView : nullptr;
	}

	m_dxDeviceContext->OMSetRenderTargets(numColorTargets, 
										  dxRenderTargets, 
										  depthStencilTarget != nullptr ? depthStencilTarget->m_dxDepthStencilView : nullptr);

	delete[] dxRenderTargets;
}

void RHIDeviceContext::ClearColorTarget(RHITextureBase* colorTarget, const Rgba& clearColor)
{
	if(!colorTarget){
		return;
	}

	if(!m_dxDeviceContext){
		return;
	}

	float colorComponentsAsFloats[4];
	clearColor.GetAsFloats(colorComponentsAsFloats);
	m_dxDeviceContext->ClearRenderTargetView(colorTarget->m_dxRenderTargetView, colorComponentsAsFloats);
}

void RHIDeviceContext::ClearDepthTarget(RHITextureBase* depthTarget, float depth, uint8_t stencil)
{
    if(nullptr != depthTarget){
    	ASSERT_OR_DIE(depthTarget->IsDepthStencilView(), "RHITexture2D must be a valid depth target to clear it");
    	m_dxDeviceContext->ClearDepthStencilView(depthTarget->m_dxDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
    }
}

void RHIDeviceContext::SetViewport(int x, int y, int width, int height)
{
	// Also, set which region of the screen we're rendering to, in this case, all of it 
	D3D11_VIEWPORT viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.TopLeftX = (FLOAT)x;
	viewport.TopLeftY = (FLOAT)y;
	viewport.Width = (FLOAT)width;
	viewport.Height = (FLOAT)height;
	viewport.MinDepth = 0.0f;        // must be between 0 and 1 (defualt is 0);
	viewport.MaxDepth = 1.0f;        // must be between 0 and 1 (default is 1)

	m_dxDeviceContext->RSSetViewports(1, &viewport);
}

void RHIDeviceContext::SetShaderProgram(ShaderProgram* shaderProgram)
{
	m_dxDeviceContext->VSSetShader(shaderProgram->m_vertex_stage->m_dx_vertex_shader, nullptr, 0);

    if(nullptr != shaderProgram->m_geometry_stage){
        m_dxDeviceContext->GSSetShader(shaderProgram->m_geometry_stage->m_dx_geometry_shader, nullptr, 0);
    }else{
        m_dxDeviceContext->GSSetShader(nullptr, nullptr, 0);
    }

	m_dxDeviceContext->PSSetShader(shaderProgram->m_fragment_stage->m_dx_pixel_shader, nullptr, 0);
	m_dxDeviceContext->IASetInputLayout(shaderProgram->m_vertex_stage->m_dx_input_layout);
}

void RHIDeviceContext::SetTexture(const unsigned int index, RHITextureBase* texture)
{
	ID3D11ShaderResourceView* srv[] = { nullptr };

	if(texture){
		srv[0] = texture->m_dxShaderResourceView;
	}

	m_dxDeviceContext->VSSetShaderResources(index, 1, srv);
	m_dxDeviceContext->GSSetShaderResources(index, 1, srv);
	m_dxDeviceContext->PSSetShaderResources(index, 1, srv);
	m_dxDeviceContext->CSSetShaderResources(index, 1, srv);
}

void RHIDeviceContext::SetSampler(const unsigned int index, Sampler* sampler)
{
	ID3D11SamplerState* dx_sampler[] = { nullptr };

	if(sampler){
		dx_sampler[0] = sampler->m_dxSampler;
	}

	m_dxDeviceContext->VSSetSamplers(index, 1, dx_sampler);
	m_dxDeviceContext->GSSetSamplers(index, 1, dx_sampler);
	m_dxDeviceContext->PSSetSamplers(index, 1, dx_sampler);
	m_dxDeviceContext->CSSetSamplers(index, 1, dx_sampler);
}

void RHIDeviceContext::SetConstantBuffer(const unsigned int index, ConstantBuffer* constantBuffer)
{
	ID3D11Buffer* dx_constant_buffer[] = { nullptr };

	if(constantBuffer){
		dx_constant_buffer[0] = constantBuffer->m_dxBuffer;
	}

    // vertex, geometry, pixel
	m_dxDeviceContext->VSSetConstantBuffers(index, 1, dx_constant_buffer);
	m_dxDeviceContext->GSSetConstantBuffers(index, 1, dx_constant_buffer);
	m_dxDeviceContext->PSSetConstantBuffers(index, 1, dx_constant_buffer);

    // compute
	m_dxDeviceContext->CSSetConstantBuffers(index, 1, dx_constant_buffer);
}

void RHIDeviceContext::SetStructuredBuffer(const unsigned int index, StructuredBuffer* structuredBuffer)
{
	ID3D11ShaderResourceView* dx_structured_buffer_srv[] = { nullptr };

	if(structuredBuffer){
		dx_structured_buffer_srv[0] = structuredBuffer->m_dx_srv;
	}

    // vertex, geometry, pixel
	m_dxDeviceContext->VSSetShaderResources(index, 1, dx_structured_buffer_srv);
	m_dxDeviceContext->GSSetShaderResources(index, 1, dx_structured_buffer_srv);
	m_dxDeviceContext->PSSetShaderResources(index, 1, dx_structured_buffer_srv);

    // compute
	m_dxDeviceContext->PSSetShaderResources(index, 1, dx_structured_buffer_srv);
}

void RHIDeviceContext::SetRasterState(RasterState* rasterState)
{
	m_dxDeviceContext->RSSetState(rasterState->m_dxRasterState);
}

void RHIDeviceContext::SetBlendState(BlendState* blendState)
{
	float constant[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_dxDeviceContext->OMSetBlendState(blendState->m_dxBlendState, constant, 0xFFFFFFFF);
}

void RHIDeviceContext::SetDepthStencilState(DepthStencilState* depthStencilState)
{
	m_dxDeviceContext->OMSetDepthStencilState(depthStencilState->m_dxDepthStencilState, 0U);
}

void RHIDeviceContext::Draw(PrimitiveType topology, VertexBuffer* vbo, const unsigned int vertexCount, const unsigned int startIndex)
{
	D3D11_PRIMITIVE_TOPOLOGY dxTopology = DXGetTopology(topology);
	m_dxDeviceContext->IASetPrimitiveTopology(dxTopology);

	unsigned int stride = sizeof(Vertex3);
	unsigned int offsets = 0;

	m_dxDeviceContext->IASetVertexBuffers(0, 1, &vbo->m_dxBuffer, &stride, &offsets);
	m_dxDeviceContext->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0U);

	m_dxDeviceContext->Draw(vertexCount, startIndex);
}

void RHIDeviceContext::DrawIndexed(PrimitiveType topology, VertexBuffer* vbo, IndexBuffer* ibo, const unsigned int indexCount, const unsigned int startIndex)
{
	D3D11_PRIMITIVE_TOPOLOGY dxTopology = DXGetTopology(topology);
	m_dxDeviceContext->IASetPrimitiveTopology(dxTopology);

	unsigned int stride = sizeof(Vertex3);
	unsigned int offsets = 0;

	m_dxDeviceContext->IASetVertexBuffers(0, 1, &vbo->m_dxBuffer, &stride, &offsets);
	m_dxDeviceContext->IASetIndexBuffer(ibo->m_dxBuffer, DXGI_FORMAT_R32_UINT, 0U);

	m_dxDeviceContext->DrawIndexed(indexCount, startIndex, 0);
}

void RHIDeviceContext::DispatchComputeJob(ComputeJob* compute_job)
{
    // set shader
    m_dxDeviceContext->CSSetShader(compute_job->m_compute_shader->m_compute_stage->m_dx_compute_shader, nullptr, 0);

    // set texture
    //#TODO: don't hardcode texture index
    m_dxDeviceContext->CSSetUnorderedAccessViews(0, 1, &compute_job->m_out_texture->m_dxUnorderedAccessView, nullptr);

    // dispatch w, h, d
    gpu_query_begin(this);
    m_dxDeviceContext->Dispatch(compute_job->m_grid_width, compute_job->m_grid_height, compute_job->m_grid_depth);
    gpu_query_end(this);

    // unbind the uav automatically so it can be used in other calls
	ID3D11UnorderedAccessView* uav[] = { nullptr };
    m_dxDeviceContext->CSSetUnorderedAccessViews(0, 1, uav, nullptr);
}