#pragma once

#include "Krystal.Gfx.OpenGL/MaterialSystem.hpp"
#include "Krystal.Gfx.OpenGL/MeshSystem.hpp"
#include "Krystal.Gfx.OpenGL/SamplerSystem.hpp"
#include "Krystal.Gfx.OpenGL/ShaderSystem.hpp"
#include "Krystal.Gfx.OpenGL/TextureSystem.hpp"
#include "Krystal.Gfx/IContext.hpp"

namespace Krys::Gfx::OpenGL
{
  class Context final : public IContext
  {
    class ContextPlatformImpl;

    NO_COPY_MOVE(Context)

    NativeHandle _windowHandle;
    uint32 _width;
    uint32 _height;
    Unique<ContextPlatformImpl> _platformImpl;

    SamplerSystem _samplers;
    TextureSystem _textures;
    ShaderSystem _shaders;
    MeshSystem _meshes;
    MaterialSystem _materials;

  public:
    Context(NativeHandle windowHandle, uint32 width, uint32 height);

    ~Context() noexcept override = default;

    void Setup() noexcept override;

    void Render(ICamera &camera) noexcept override;

    void Present() noexcept override;

    void Resize(uint32 width, uint32 height) noexcept override;

    ISamplerSystem &Samplers() noexcept override;

    ITextureSystem &Textures() noexcept override;

    IShaderSystem &Shaders() noexcept override;

    IMeshSystem &Meshes() noexcept override;

    IMaterialSystem &Materials() noexcept override;
  };
}
