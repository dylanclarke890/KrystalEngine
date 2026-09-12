#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/IO/Path.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Shader.hpp"

namespace krys::Gfx
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
    Font_MSDF_Outline, // TODO(check): are outlines are suitable for MSDF?
    Font_MTSDF,
    Font_MTSDF_Outline,
  };

  class IShaderRegistry
  {
    KRYS_NON_COPY_MOVABLE(IShaderRegistry);

  protected:
    IShaderRegistry() noexcept = default;

  public:
    virtual ~IShaderRegistry() noexcept = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    KRYS_NODISCARD virtual ShaderHandle Load(const io::Path &vertex, const io::Path &fragment) noexcept = 0;

    KRYS_NODISCARD virtual ShaderHandle Load(const io::Path &vertex, const io::Path &geometry,
                                             const io::Path &fragment) noexcept = 0;

    KRYS_NODISCARD virtual ShaderHandle GetBuiltin(BuiltinShader shader) noexcept = 0;

    virtual bool Unload(ShaderHandle handle) noexcept = 0;
  };
}