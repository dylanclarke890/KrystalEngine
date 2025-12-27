#pragma once

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
#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.IO/VirtualFileSystem.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"

namespace Krys::Gfx::OpenGL
{
  class Context final : public IContext, NonCopyMovable<Context>
  {
    class ContextPlatformImpl;

    NativeHandle _windowHandle;
    uint32 _width;
    uint32 _height;
    IO::VirtualFileSystem &_vfs;
    StringInterner &_strings;
    int _dpi;
    UniquePtr<ContextPlatformImpl> _platformImpl;

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

    KRYS_NODISCARD uint32 Width() const noexcept;

    KRYS_NODISCARD uint32 Height() const noexcept;

    KRYS_NODISCARD IBufferRegistry &Buffers() noexcept override;

    KRYS_NODISCARD IImageRegistry &Images() noexcept override;

    KRYS_NODISCARD IImageViewRegistry &ImageViews() noexcept override;

    KRYS_NODISCARD ISamplerRegistry &Samplers() noexcept override;

    KRYS_NODISCARD ITextureRegistry &Textures() noexcept override;

    KRYS_NODISCARD IRenderTargetRegistry &RenderTargets() noexcept override;

    KRYS_NODISCARD IShaderRegistry &Shaders() noexcept override;

    KRYS_NODISCARD IMeshRegistry &Meshes() noexcept override;

    KRYS_NODISCARD IMaterialRegistry &Materials() noexcept override;

    KRYS_NODISCARD IFontRegistry &Fonts() noexcept override;

    KRYS_NODISCARD StringInterner &Strings() noexcept override;

    KRYS_NODISCARD API GetAPI() const noexcept override;
  };
}
