#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Mesh.hpp"
#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"

namespace Krys::Gfx
{
  class IMeshRegistry : NonCopyMovable<IMeshRegistry>
  {
  protected:
    IMeshRegistry() noexcept = default;

  public:
    virtual ~IMeshRegistry() noexcept = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    KRYS_NODISCARD virtual MeshHandle Create(const MeshDesc &desc) noexcept = 0;

    KRYS_NODISCARD virtual MeshHandle GetFullScreenQuad() const noexcept = 0;

    virtual bool Destroy(MeshHandle handle) noexcept = 0;
  };
}