#pragma once

#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class Color;

  /// @brief Who a colour chooser tells about the user's choice: a colour input. It ends its chooser before it
  /// goes away.
  class ColorChooserClient
  {
  public:
    virtual ~ColorChooserClient() noexcept = default;

    virtual void DidChooseColor(const Color &color) noexcept = 0;
    virtual void DidEndChooser() noexcept = 0;
    KRYS_NODISCARD virtual IntRect ElementRectRelativeToRootView() const noexcept = 0;
    KRYS_NODISCARD virtual bool SupportsAlpha() const noexcept = 0;
    KRYS_NODISCARD virtual List<Color> SuggestedColors() const noexcept = 0;
  };
}
