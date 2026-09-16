#pragma once

#include "Krystal.Core/Commands/CommandList.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Gfx/IContext.hpp"

namespace krys::Gfx
{
  Expected<UniquePtr<class IRenderer>> CreateRenderer(IContext &ctx) noexcept;

  class IRenderer
  {
    KRYS_NON_COPY_MOVABLE(IRenderer);

  public:
    IRenderer() noexcept = default;

    virtual ~IRenderer() = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    virtual void BeginFrame() = 0;

    virtual void EndFrame() = 0;

    virtual void Submit(const CommandList &commandList) = 0;
  };
}