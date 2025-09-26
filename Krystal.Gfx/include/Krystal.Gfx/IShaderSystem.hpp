#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/TextShaderDesc.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Attributes.hpp"

namespace Krys::Gfx
{
  class IShaderSystem
  {
  public:
    virtual ~IShaderSystem() = default;

    NO_DISCARD virtual ShaderHandle Load(const IO::Path &vertex, const IO::Path &fragment) noexcept = 0;

    NO_DISCARD virtual ShaderHandle Load(const IO::Path &vertex, const IO::Path &geometry,
                                         const IO::Path &fragment) noexcept = 0;

    NO_DISCARD virtual ShaderHandle LoadTextShader(const IO::Path &vertex, const IO::Path &fragment,
                                                   const TextShaderDesc &desc) noexcept = 0;

    virtual void Unload(ShaderHandle handle) noexcept = 0;
  };
}