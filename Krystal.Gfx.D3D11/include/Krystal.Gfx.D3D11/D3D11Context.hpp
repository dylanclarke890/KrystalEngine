#pragma once

#include "Krystal.Core/IO/VirtualFileSystem.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

namespace krys::Gfx::D3D11
{
  class D3D11Context : public IContext
  {
  private:
    HWND _windowHandle;
    io::VirtualFileSystem &_vfs;
    IDXGISwapChain *_swapchain;
    ID3D11Device *_device;
    ID3D11DeviceContext *_context;
    ID3D11RenderTargetView *_backbuffer;

  public:
    D3D11Context(const ContextSettings &settings);
    ~D3D11Context() noexcept;

    void Setup() noexcept;

    void Render(ICamera &camera) noexcept;

    void Present() noexcept;

    virtual void Resize(uint32 width, uint32 height) noexcept;
  };
}
