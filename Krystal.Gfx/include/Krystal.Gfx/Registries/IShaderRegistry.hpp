#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Shader.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Macros.hpp"

namespace Krys::Gfx
{
  enum class BuiltinShader
  {
    Shape2D_Colour,
    Shape2D_Texture,
    PostProcess_Passthrough,
    Font_Bitmap,
    Font_SDF,
    Font_SDF_Outline,
    Font_MSDF,
    Font_MSDF_Outline, // TODO: check if outlines are suitable for MSDF
    Font_MTSDF,
    Font_MTSDF_Outline,
  };

  class IShaderRegistry
  {
    NO_COPY_MOVE(IShaderRegistry)

  protected:
    IShaderRegistry() noexcept = default;

  public:
    virtual ~IShaderRegistry() noexcept = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    NO_DISCARD virtual ShaderHandle Load(const IO::Path &vertex, const IO::Path &fragment) noexcept = 0;

    NO_DISCARD virtual ShaderHandle Load(const IO::Path &vertex, const IO::Path &geometry,
                                         const IO::Path &fragment) noexcept = 0;

    NO_DISCARD virtual ShaderHandle GetBuiltin(BuiltinShader shader) noexcept = 0;

    virtual bool Unload(ShaderHandle handle) noexcept = 0;
  };
}