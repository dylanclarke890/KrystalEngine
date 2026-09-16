#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Mesh.hpp"
#include "Krystal.Gfx/Vertex.hpp"

namespace krys::Gfx
{
  class IMeshRegistry
  {
    KRYS_NON_COPY_MOVABLE(IMeshRegistry);

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