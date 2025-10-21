#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Shader.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Macros.hpp"

namespace Krys::Gfx
{
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

    NO_DISCARD virtual ShaderHandle LoadTextShader(const IO::Path &vertex, const IO::Path &fragment,
                                                   const TextShaderDesc &desc) noexcept = 0;

    virtual bool Unload(ShaderHandle handle) noexcept = 0;
  };
}