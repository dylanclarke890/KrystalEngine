#include "Krystal.Gfx.D3D12/Context.hpp"

namespace Krys::Gfx
{
  D3D12Context::D3D12Context(NativeHandle windowHandle) : _windowHandle(windowHandle.As<HWND>())
  {
  }

  void D3D12Context::SetupTestTriangle() noexcept
  {
  }

  void D3D12Context::DrawTestTriangle() noexcept
  {
  }

  void D3D12Context::Present() noexcept
  {
  }
}