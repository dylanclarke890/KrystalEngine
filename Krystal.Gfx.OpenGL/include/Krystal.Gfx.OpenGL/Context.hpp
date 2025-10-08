#pragma once

#include "Krystal.Gfx.OpenGL/Fonts/FontRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Images/ImageRegistry.hpp"
#include "Krystal.Gfx.OpenGL/ImageViews/ImageViewRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Materials/MaterialRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Meshes/MeshRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Samplers/SamplerRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Shaders/ShaderRegistry.hpp"
#include "Krystal.Gfx.OpenGL/TextRenderer.hpp"
#include "Krystal.Gfx.OpenGL/Textures/TextureRegistry.hpp"
#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.IO/VirtualFileSystem.hpp"

namespace Krys::Gfx::OpenGL
{
  class Context final : public IContext
  {
    class ContextPlatformImpl;

    NO_COPY_MOVE(Context)

    NativeHandle _windowHandle;
    uint32 _width;
    uint32 _height;
    IO::VirtualFileSystem &_vfs;
    int _dpi;
    Unique<ContextPlatformImpl> _platformImpl;

    ImageRegistry _images;
    ImageViewRegistry _imageViews;
    SamplerRegistry _samplers;
    ShaderRegistry _shaders;
    MeshRegistry _meshes;
    TextureRegistry _textures;
    MaterialRegistry _materials;
    FontRegistry _fonts;
    TextRenderer _text;

  public:
    Context(const ContextSettings &settings);

    ~Context() noexcept override = default;

    void Setup() noexcept override;

    void Render(ICamera &camera) noexcept override;

    void Present() noexcept override;

    void Resize(uint32 width, uint32 height) noexcept override;

    void DPIChanged(int dpi) noexcept override;

    NO_DISCARD IImageRegistry &Images() noexcept override;

    NO_DISCARD IImageViewRegistry &ImageViews() noexcept override;

    NO_DISCARD ISamplerRegistry &Samplers() noexcept override;

    NO_DISCARD ITextureRegistry &Textures() noexcept override;

    NO_DISCARD IShaderRegistry &Shaders() noexcept override;

    NO_DISCARD IMeshRegistry &Meshes() noexcept override;

    NO_DISCARD IMaterialRegistry &Materials() noexcept override;

    NO_DISCARD IFontRegistry &Fonts() noexcept override;

    NO_DISCARD API GetAPI() const noexcept override;
  };
}
