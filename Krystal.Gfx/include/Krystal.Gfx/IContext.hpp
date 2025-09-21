#pragma once

#include "Krystal.Gfx/ICamera.hpp"
#include "Krystal.IO/Image.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/NativeHandle.hpp"
#include "Krystal.Lib/SmartPointers.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
  class IContext;
  class ITextureSystem;
  class IShaderSystem;
  class IMeshSystem;

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

    virtual void Present() noexcept = 0;

    virtual void Resize(uint32 width, uint32 height) noexcept = 0;

    virtual ITextureSystem &Textures() noexcept = 0;

    virtual IShaderSystem &Shaders() noexcept = 0;

    virtual IMeshSystem &Meshes() noexcept = 0;
  };
}