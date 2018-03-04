#include "Engine/RHI/RHIOutput.hpp"
#include "Engine/RHI/RHITexture2D.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/DX11.hpp"

#include "Engine/Core/Window.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

static WindowMode GetWindowModeForRHIOutputMode(RHIOutputMode rhiOutputMode)
{
	switch(rhiOutputMode){
		case RHI_OUTPUT_MODE_WINDOWED:					return WINDOW_MODE_WINDOWED;				break;
		case RHI_OUTPUT_MODE_BORDERLESS:				return WINDOW_MODE_BORDERLESS;				break;
		case RHI_OUTPUT_MODE_FULLSCREEN_WINDOW:			return WINDOW_MODE_FULLSCREEN_WINDOWED;		break;
		/*case RHI_OUTPUT_MODE_FULLSCREEN_DEDICATED:		return WINDOW_MODE_FULLSCREEN_DEDICATED;	break;*/
		default:										return WINDOW_MODE_WINDOWED;				break;
	}
}

RHIOutput::RHIOutput(RHIDevice *owner, Window *window, IDXGISwapChain* dx11SwapChain)
	:m_device(owner)
	,m_window(window)
	,m_dxSwapChain(dx11SwapChain)
	,m_renderTarget(nullptr)
{
	ASSERT_OR_DIE(owner, "RHIOutput was created with a null RHIDevice owner.\n");
	ASSERT_OR_DIE(window, "RHIOutput was created with a null window.\n");
	ASSERT_OR_DIE(dx11SwapChain, "RHIOutput was created with a null DirectX11 Swap Chain.\n");
	CreateRenderTarget();
}

RHIOutput::~RHIOutput()
{
	DX_SAFE_RELEASE(m_dxSwapChain);
	SAFE_DELETE(m_renderTarget);
	SAFE_DELETE(m_window);
}

void RHIOutput::Present()
{
	if(m_window->IsOpen()){
		m_dxSwapChain->Present(0, 0);
	}
}

void RHIOutput::Close()
{
	m_window->Close();
}

unsigned int RHIOutput::GetWidth() const
{
	return m_window->GetWidth();
}

unsigned int RHIOutput::GetHeight() const
{
	return m_window->GetHeight();
}

float RHIOutput::GetAspectRatio() const
{
	return m_window->GetAspectRatio();
}

bool RHIOutput::IsOpen() const
{
	return m_window->IsOpen();
}

void RHIOutput::SetDisplayMode(const RHIOutputMode rhiOutputMode)
{
	WindowMode windowMode = GetWindowModeForRHIOutputMode(rhiOutputMode);
	m_window->SetWindowMode(windowMode);

	SAFE_DELETE(m_renderTarget);
	HRESULT hr = m_dxSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	if(!SUCCEEDED(hr)){
		return;
	}

	CreateRenderTarget();
}

bool RHIOutput::SetDisplaySize(int width, int height)
{
	if (m_window->GetWindowMode() == WINDOW_MODE_FULLSCREEN_WINDOWED){
		return false;
	}

	SAFE_DELETE(m_renderTarget);
	m_window->SetClientSize(width, height);

	HRESULT hr = m_dxSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	if(!SUCCEEDED(hr)){
		return false;
	}

	CreateRenderTarget();

	return true;
}

void RHIOutput::SetDisplayTitle(const char* title)
{
	m_window->SetTitle(title);
}

void RHIOutput::CenterDisplay()
{
	m_window->Center();
}

void RHIOutput::ProcessMessages()
{
	m_window->ProcessMessages();
}

void RHIOutput::ConfigureWindow(Window** out_window, int width, int height, RHIOutputMode rhiOutputMode)
{
	WindowMode windowMode = GetWindowModeForRHIOutputMode(rhiOutputMode);
	*out_window = new Window(width, height, windowMode);
}

void RHIOutput::CreateRenderTarget()
{
	m_renderTarget = new RHITexture2D(m_device, this);
}