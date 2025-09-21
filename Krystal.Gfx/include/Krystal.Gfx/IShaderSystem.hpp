#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.IO/Path.hpp"

namespace Krys::Gfx
{
  struct ShaderHandle : Handle<ShaderHandle>
  {
  };

  class IShaderSystem
  {
  public:
    virtual ~IShaderSystem() = default;

    NO_DISCARD virtual ShaderHandle Load(const IO::Path &vertex, const IO::Path &fragment) noexcept = 0;

    NO_DISCARD virtual ShaderHandle Load(const IO::Path &vertex, const IO::Path &geometry,
                                         const IO::Path &fragment) noexcept = 0;

    virtual void Unload(ShaderHandle handle) noexcept = 0;
  };
}