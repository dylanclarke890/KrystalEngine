#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Mesh.hpp"
#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Span.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
  class IMeshRegistry
  {
    NO_COPY_MOVE(IMeshRegistry)

  protected:
    IMeshRegistry() noexcept = default;

  public:
    virtual ~IMeshRegistry() noexcept = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    NO_DISCARD virtual MeshHandle Create(const MeshDesc& desc) = 0;

    virtual bool Destroy(MeshHandle handle) noexcept = 0;
  };
}