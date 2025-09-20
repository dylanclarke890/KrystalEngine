#pragma once

#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.IO/Image.hpp"

#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

namespace Krys::Gfx::D3D11
{
  class D3D11Context final : public IContext
  {
    HWND _windowHandle;
    IDXGISwapChain *_swapchain;
    ID3D11Device *_device;
    ID3D11DeviceContext *_context;
    ID3D11RenderTargetView *_backbuffer;

    NO_COPY_MOVE(D3D11Context)
  public:
    D3D11Context(NativeHandle windowHandle, uint32 width, uint32 height);
    ~D3D11Context() noexcept override;

    void Setup() noexcept override;

    void Render(ICamera& camera) noexcept override;

    void Present() noexcept override;

    virtual void Resize(uint32 width, uint32 height) noexcept override;
  };
}
