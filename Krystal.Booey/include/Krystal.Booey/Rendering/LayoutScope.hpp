#pragma once

#include "Krystal.Core/Macros.hpp"

namespace krys::boo
{
  class RenderElement;

  /// @brief Finishes a renderer's layout when it goes out of scope: updates a block's scroll information and
  /// clears the renderer's layout bits.
  class LayoutScope
  {
    KRYS_NON_COPY_MOVABLE(LayoutScope);

  private:
    RenderElement &_renderer;

  public:
    explicit LayoutScope(RenderElement &renderer) noexcept;
    ~LayoutScope() noexcept;
  };
}
