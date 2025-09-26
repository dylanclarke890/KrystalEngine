#pragma once

#include "Krystal.Gfx.OpenGL/Fonts/FontRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Materials/MaterialRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Meshes/MeshRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Samplers/SamplerRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Shaders/ShaderRegistry.hpp"
#include "Krystal.Gfx.OpenGL/TextRenderer.hpp"
#include "Krystal.Gfx.OpenGL/Textures/TextureRegistry.hpp"
#include "Krystal.Gfx/Colour.hpp"
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
    int _dpi;
    Unique<ContextPlatformImpl> _platformImpl;

    TextureRegistry _textures;
    SamplerRegistry _samplers;
    ShaderRegistry _shaders;
    MeshRegistry _meshes;
    MaterialRegistry _materials;
    FontRegistry _fonts;
    TextRenderer _text;

  public:
    Context(NativeHandle windowHandle, uint32 width, uint32 height);

    ~Context() noexcept override = default;

    void Setup() noexcept override;

    void Render(ICamera &camera) noexcept override;

    void Present() noexcept override;

    void Resize(uint32 width, uint32 height) noexcept override;

    void DPIChanged(int dpi) noexcept override;

    ISamplerRegistry &Samplers() noexcept override;

    ITextureRegistry &Textures() noexcept override;

    IShaderRegistry &Shaders() noexcept override;

    IMeshRegistry &Meshes() noexcept override;

    IMaterialRegistry &Materials() noexcept override;

    IFontRegistry &Fonts() noexcept override;

    void DrawText(FontHandle font, const string &text, const Maths::Vec2 &position,
                  Colour textColour = Colours::White) noexcept;
  };
}
