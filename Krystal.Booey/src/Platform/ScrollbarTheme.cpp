#include "Krystal.Booey/Platform/ScrollbarTheme.hpp"

namespace krys::boo
{
  ScrollbarTheme &ScrollbarTheme::Theme() noexcept
  {
    static ScrollbarTheme theme;
    return theme;
  }

  int32 ScrollbarTheme::ScrollbarThickness(ScrollbarWidth width,
                                           OverlayScrollbarSizeRelevancy relevancy) const noexcept
  {
    if (UsesOverlayScrollbars() && relevancy == OverlayScrollbarSizeRelevancy::IgnoreOverlayScrollbarSize)
    {
      return 0;
    }

    switch (width)
    {
      case ScrollbarWidth::Auto: return 15;
      case ScrollbarWidth::Thin: return 11;
      case ScrollbarWidth::None: return 0;
    }
    krys_debug_assert(false);
    return 15;
  }
}
