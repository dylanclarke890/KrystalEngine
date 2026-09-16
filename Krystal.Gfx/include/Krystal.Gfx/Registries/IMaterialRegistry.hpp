#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Material.hpp"

namespace krys::Gfx
{
  class IMaterialRegistry
  {
    KRYS_NON_COPY_MOVABLE(IMaterialRegistry);

  protected:
    IMaterialRegistry() noexcept = default;

  public:
    virtual ~IMaterialRegistry() noexcept = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    KRYS_NODISCARD virtual MaterialHandle Create(const string &name, ShaderHandle shader,
                                                 const PBRMaterialDesc &desc) noexcept = 0;

    virtual bool Unload(MaterialHandle handle) noexcept = 0;
  };
}