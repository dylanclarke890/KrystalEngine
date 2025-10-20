#pragma once

#include "Krystal.Gfx.Lib/Colour.hpp"
#include "Krystal.Gfx/Common.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
  class IRenderTargetRegistry
  {
    NO_COPY_MOVE(IRenderTargetRegistry)

  public:
    IRenderTargetRegistry() = default;

    virtual ~IRenderTargetRegistry() = default;

    virtual void Startup() = 0;
    virtual void Shutdown() noexcept = 0;

    /// @brief Creates a new render target based on the provided description.
    virtual NO_DISCARD RenderTargetHandle Create(const RenderTargetDesc &desc) = 0;

    /// @brief Destroys the render target associated with the given handle.
    virtual void Destroy(RenderTargetHandle handle) noexcept = 0;

    /// @brief Gets the current frame's screen render target.
    virtual NO_DISCARD RenderTargetHandle GetScreenRenderTarget() const noexcept = 0;
  };
}