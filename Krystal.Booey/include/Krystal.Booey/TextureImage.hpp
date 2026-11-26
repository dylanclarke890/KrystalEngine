#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Gfx/Handles.hpp"
#include "Krystal.Web/Graphics/Images/PlatformImage.hpp"

namespace krys::boo
{
  /// @brief An image Krystal.Booey holds as a texture on the view's device, drawn by the renderer without a
  /// copy: an engine texture, or a snapshot of an accelerated image buffer.
  /// @note Krystal.Booey makes every external image Krystal.Web draws, so each is one of these.
  class TextureImage : public web::ExternalImage
  {
  public:
    /// @brief The texture; invalid once it's gone.
    KRYS_NODISCARD virtual gfx::TextureHandle Handle() const noexcept = 0;
  };
}
