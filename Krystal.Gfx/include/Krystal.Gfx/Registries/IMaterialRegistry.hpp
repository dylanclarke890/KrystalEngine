#pragma once

#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Material.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/String/String.hpp"

namespace Krys::Gfx
{
  class IMaterialRegistry
  {
    NO_COPY_MOVE(IMaterialRegistry)

  protected:
    IMaterialRegistry() noexcept = default;

  public:
    virtual ~IMaterialRegistry() noexcept = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    NO_DISCARD virtual MaterialHandle Create(const string &name, ShaderHandle shader,
                                             const PBRMaterialDesc &desc) noexcept = 0;

    virtual bool Unload(MaterialHandle handle) noexcept = 0;
  };
}