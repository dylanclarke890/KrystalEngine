#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.IO/Image.hpp"
#include "Krystal.Gfx/ICamera.hpp"

namespace Krys::Gfx
{
  class IContext;

  Expected<Unique<IContext>> CreateContext(NativeHandle windowHandle, uint32 width, uint32 height) noexcept;

  enum class API
  {
    None = 0,
    OpenGL,
    D3D11,
    D3D12,
    Vulkan,
  };

  class IContext
  {
    NO_COPY_MOVE(IContext)

  protected:
    IContext() noexcept = default;

  public:
    virtual ~IContext() noexcept = default;

    virtual void Setup() noexcept = 0;

    virtual void Render(ICamera &camera) noexcept = 0;

    virtual void SetSkybox(const IO::CubeMapImage &skybox) noexcept = 0;

    virtual void Present() noexcept = 0;

    virtual void Resize(uint32 width, uint32 height) noexcept = 0;
  };
}