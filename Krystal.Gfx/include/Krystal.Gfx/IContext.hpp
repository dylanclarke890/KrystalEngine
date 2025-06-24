#pragma once

#include "Krystal.Core/Core.hpp"

namespace Krys::Gfx
{
  class IContext;

  Expected<Unique<IContext>> CreateContext(NativeHandle windowHandle, uint32 width, uint32 height) noexcept;

  enum class API
  {
    None = 0,
    OpenGL = 1,
    D3D11 = 2,
  };

  class IContext
  {
    NO_COPY_MOVE(IContext)

  protected:
    IContext() noexcept = default;

  public:
    virtual ~IContext() noexcept = default;

    virtual void SetupTestTriangle() noexcept = 0;

    virtual void DrawTestTriangle() noexcept = 0;

    virtual void Present() noexcept = 0;

    virtual void Resize(uint32 width, uint32 height) noexcept = 0;
  };
}