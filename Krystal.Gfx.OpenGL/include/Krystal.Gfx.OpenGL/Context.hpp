#pragma once

#include "Krystal.Gfx.OpenGL/Fonts/FontSystem.hpp"
#include "Krystal.Gfx.OpenGL/MaterialSystem.hpp"
#include "Krystal.Gfx.OpenGL/MeshSystem.hpp"
#include "Krystal.Gfx.OpenGL/SamplerSystem.hpp"
#include "Krystal.Gfx.OpenGL/ShaderSystem.hpp"
#include "Krystal.Gfx.OpenGL/TextureSystem.hpp"
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

    TextureSystem _textures;
    SamplerSystem _samplers;
    ShaderSystem _shaders;
    MeshSystem _meshes;
    MaterialSystem _materials;
    FontSystem _fonts;

  public:
    Context(NativeHandle windowHandle, uint32 width, uint32 height);

    ~Context() noexcept override = default;

    void Setup() noexcept override;

    void Render(ICamera &camera) noexcept override;

    void Present() noexcept override;

    void Resize(uint32 width, uint32 height) noexcept override;

    void DPIChanged(int dpi) noexcept override;

    ISamplerSystem &Samplers() noexcept override;

    ITextureSystem &Textures() noexcept override;

    IShaderSystem &Shaders() noexcept override;

    IMeshSystem &Meshes() noexcept override;

    IMaterialSystem &Materials() noexcept override;

    IFontSystem &Fonts() noexcept override;

    void DrawText(FontHandle font, const string &text, const Maths::Vec2 &position,
                  Colour textColour = Colours::White) noexcept;
  };
}
