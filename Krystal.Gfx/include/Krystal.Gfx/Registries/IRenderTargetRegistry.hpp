#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/RenderTarget.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Maths/Vector.hpp"

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

    /// @brief Destroys a render target.
    virtual bool Destroy(RenderTargetHandle handle) noexcept = 0;

    /// @brief Acquires a render target matching the provided description from the cache, or creates a new one
    /// if none are available.
    virtual NO_DISCARD RenderTargetHandle Acquire(const RenderTargetDesc &desc) = 0;

    /// @brief Releases a previously acquired render target back to the cache.
    virtual void Release(RenderTargetHandle handle) noexcept = 0;

    virtual void ClearCache() noexcept = 0;

    /// @brief Get the dimensions of a render target.
    virtual Maths::Vec2 GetDimensions(RenderTargetHandle handle) noexcept = 0;

    /// @brief Gets the current frame's screen render target.
    virtual NO_DISCARD RenderTargetHandle GetScreenRenderTarget() const noexcept = 0;
  };
}