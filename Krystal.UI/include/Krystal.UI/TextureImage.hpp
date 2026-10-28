#pragma once

#include "Krystal.Booey/Graphics/PlatformImage.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Gfx/Handles.hpp"

namespace krys::ui
{
  /// @brief An image Krystal.UI holds as a texture on the view's device, drawn by the renderer without a
  /// copy: an engine texture, or a snapshot of an accelerated image buffer.
  /// @note Krystal.UI makes every external image Booey draws, so each is one of these.
  class TextureImage : public boo::ExternalImage
  {
  public:
    /// @brief The texture; invalid once it's gone.
    KRYS_NODISCARD virtual gfx::TextureHandle Handle() const noexcept = 0;
  };
}
