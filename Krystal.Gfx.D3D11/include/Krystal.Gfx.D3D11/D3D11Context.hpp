#pragma once

#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.IO/VirtualFileSystem.hpp"
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

namespace Krys::Gfx::D3D11
{
  class D3D11Context final
  {
    HWND _windowHandle;
    IO::VirtualFileSystem &_vfs;
    IDXGISwapChain *_swapchain;
    ID3D11Device *_device;
    ID3D11DeviceContext *_context;
    ID3D11RenderTargetView *_backbuffer;

    NO_COPY_MOVE(D3D11Context)
  public:
    D3D11Context(const ContextSettings &settings);
    ~D3D11Context() noexcept;

    void Setup() noexcept;

    void Render(ICamera &camera) noexcept;

    void Present() noexcept;

    virtual void Resize(uint32 width, uint32 height) noexcept;
  };
}
