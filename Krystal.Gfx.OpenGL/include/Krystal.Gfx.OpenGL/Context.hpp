#pragma once

#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Gfx.OpenGL/Registries/BufferRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Registries/FontRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Registries/ImageRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Registries/ImageViewRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Registries/MaterialRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Registries/MeshRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Registries/RenderTargetRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Registries/SamplerRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Registries/ShaderRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Registries/TextureRegistry.hpp"
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
    StringInterner& _strings;
    int _dpi;
    Unique<ContextPlatformImpl> _platformImpl;

    BufferRegistry _buffers;
    ImageRegistry _images;
    ImageViewRegistry _imageViews;
    SamplerRegistry _samplers;
    ShaderRegistry _shaders;
    MeshRegistry _meshes;
    TextureRegistry _textures;
    RenderTargetRegistry _renderTargets;
    MaterialRegistry _materials;
    FontRegistry _fonts;

  public:
    Context(const ContextSettings &settings);

    ~Context() noexcept override = default;

    void Startup() noexcept override;

    void Shutdown() noexcept override;

    void Render(ICamera &camera) noexcept override;

    void Present() noexcept override;

    void Resize(uint32 width, uint32 height) noexcept override;

    void DPIChanged(int dpi) noexcept override;

    NO_DISCARD uint32 Width() const noexcept;

    NO_DISCARD uint32 Height() const noexcept;

    NO_DISCARD IBufferRegistry &Buffers() noexcept override;

    NO_DISCARD IImageRegistry &Images() noexcept override;

    NO_DISCARD IImageViewRegistry &ImageViews() noexcept override;

    NO_DISCARD ISamplerRegistry &Samplers() noexcept override;

    NO_DISCARD ITextureRegistry &Textures() noexcept override;

    NO_DISCARD IRenderTargetRegistry &RenderTargets() noexcept override;

    NO_DISCARD IShaderRegistry &Shaders() noexcept override;

    NO_DISCARD IMeshRegistry &Meshes() noexcept override;

    NO_DISCARD IMaterialRegistry &Materials() noexcept override;

    NO_DISCARD IFontRegistry &Fonts() noexcept override;
    
    NO_DISCARD StringInterner &Strings() noexcept override;

    NO_DISCARD API GetAPI() const noexcept override;
  };
}
