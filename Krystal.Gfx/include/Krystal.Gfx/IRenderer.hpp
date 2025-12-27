#pragma once

#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Lib/Commands/CommandList.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/Types/Expected.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/SmartPointers.hpp"

namespace Krys::Gfx
{
  class IRenderer : NonCopyMovable<IRenderer>
  {
  public:
    IRenderer() noexcept = default;

    virtual ~IRenderer() = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    virtual void BeginFrame() = 0;

    virtual void EndFrame() = 0;

    virtual void Submit(const CommandList &commandList) = 0;
  };

  Expected<Unique<IRenderer>> CreateRenderer(IContext &ctx) noexcept;
}