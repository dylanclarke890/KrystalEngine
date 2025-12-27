#pragma once

#include "Krystal.Gfx/Commands/CommandList.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Lib/Types/Expected.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/SmartPointers.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::Gfx
{
  class IRenderer
  {
    NO_COPY_MOVE(IRenderer)

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