#pragma once

#include "Engine/RHI/RHITypes.hpp"

class RHIDevice;
class Window;
struct IDXGISwapChain;
class RHITexture2D;
class RHITextureBase;

class RHIOutput
{
public:
	RHIDevice*			m_device;
	Window*				m_window;
	IDXGISwapChain*		m_dxSwapChain;
	RHITextureBase*		m_renderTarget;

public:
	RHIOutput(RHIDevice *owner, Window *window, IDXGISwapChain* dx11SwapChain);
	~RHIOutput();

	void Present();
	void Close();

public:
	unsigned int	GetWidth() const;
	unsigned int	GetHeight() const;
	float			GetAspectRatio() const;
	bool			IsOpen() const;

	//RHIDevice*		GetRHIDevice() { return m_device; }
	//Window*			GetWindow() { return m_window; }
	//RHITexture2D*	GetRenderTarget() { return m_renderTarget; }

public:
	void			SetDisplayMode(const RHIOutputMode rhiDisplayMode);
	bool			SetDisplaySize(int width, int height);
	void			SetDisplayTitle(const char* title);
	void			CenterDisplay();
	void			ProcessMessages();

public:
	static void		ConfigureWindow(Window** out_window, int width, int height, RHIOutputMode outputMode = RHI_OUTPUT_MODE_WINDOWED);

private:
	void			CreateRenderTarget();
};