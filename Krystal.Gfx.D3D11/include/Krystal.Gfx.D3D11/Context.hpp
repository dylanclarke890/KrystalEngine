#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Gfx/IContext.hpp"

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
    D3D11Context(NativeHandle windowHandle);
    ~D3D11Context() noexcept override;

    void SetupTestTriangle() noexcept override;

    void DrawTestTriangle() noexcept override;

    void Present() noexcept override;
  };
}
