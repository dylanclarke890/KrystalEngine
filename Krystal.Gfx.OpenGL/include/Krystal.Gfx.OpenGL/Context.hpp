#pragma once

#include "Krystal.Gfx.OpenGL/TextureSystem.hpp"
#include "Krystal.Gfx/IContext.hpp"

namespace Krys::Gfx::OpenGL
{
  class Context final : public IContext
  {
    NO_COPY_MOVE(Context)

    NativeHandle _windowHandle;
    uint32 _width;
    uint32 _height;

    class ContextPlatformImpl;
    Unique<ContextPlatformImpl> _platformImpl;
    TextureSystem _textures;

  public:
    Context(NativeHandle windowHandle, uint32 width, uint32 height);

    ~Context() noexcept override = default;

    void Setup() noexcept override;

    void Render(ICamera &camera) noexcept override;

    void Present() noexcept override;

    void Resize(uint32 width, uint32 height) noexcept override;

    ITextureSystem &Textures() noexcept override;
  };
}
