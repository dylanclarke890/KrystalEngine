#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Gfx/IContext.hpp"

#include <d3d12.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace Krys::Gfx
{
  class D3D12Context final : public IContext
  {
    NO_COPY_MOVE(D3D12Context)

    HWND _windowHandle;

  public:
    D3D12Context(NativeHandle windowHandle);

    ~D3D12Context() noexcept override = default;

    void SetupTestTriangle() noexcept override;

    void DrawTestTriangle() noexcept override;

    void Present() noexcept override;
  };
}