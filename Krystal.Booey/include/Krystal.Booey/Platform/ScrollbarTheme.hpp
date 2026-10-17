#pragma once

#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo
{
  /// @brief How scrollbars look and how much space they take.
  /// @note The only theme has classic (non-overlay) scrollbars of a fixed thickness, as the mock theme
  /// upstream uses for layout tests.
  // FIXME: Painting, the scrollbar parts (buttons, track, thumb), hit testing and the platform themes come
  // with painting and input.
  class ScrollbarTheme
  {
    KRYS_NON_COPY_MOVABLE(ScrollbarTheme);

  public:
    ScrollbarTheme() noexcept = default;
    virtual ~ScrollbarTheme() noexcept = default;

    KRYS_NODISCARD static ScrollbarTheme &Theme() noexcept;

    KRYS_NODISCARD virtual int32
      ScrollbarThickness(ScrollbarWidth width = ScrollbarWidth::Auto,
                         OverlayScrollbarSizeRelevancy relevancy =
                           OverlayScrollbarSizeRelevancy::IncludeOverlayScrollbarSize) const noexcept;

    KRYS_NODISCARD virtual bool UsesOverlayScrollbars() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual int32 MaxOverlapBetweenPages() const noexcept
    {
      return std::numeric_limits<int32>::max();
    }
  };
}
