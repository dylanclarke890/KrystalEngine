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

    /// @brief Get the dimensions of a render target.
    virtual NO_DISCARD Maths::Vec2 GetDimensions(RenderTargetHandle handle) noexcept = 0;

    /// @brief Gets the image view handle for a specific colour attachment of a render target.
    virtual NO_DISCARD ImageViewHandle GetColourAttachmentImageView(RenderTargetHandle handle,
                                                                    uint32 index) noexcept = 0;

    /// @brief Gets the current frame's screen render target.
    virtual NO_DISCARD RenderTargetHandle GetScreenRenderTarget() const noexcept = 0;
  };
}