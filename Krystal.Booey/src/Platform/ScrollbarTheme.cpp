#include "Krystal.Booey/Platform/ScrollbarTheme.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Platform/ScrollbarThemeAdwaita.hpp"

namespace krys::boo
{
  ScrollbarTheme &ScrollbarTheme::Theme() noexcept
  {
    // FIXME: Mock scrollbars (mockScrollbarsEnabled), if they're ever wanted.
    static ScrollbarThemeAdwaita theme;
    return theme;
  }

  void ScrollbarTheme::DefaultPaintScrollCorner(ScrollableArea &, GraphicsContext &context,
                                                const IntRect &cornerRect) noexcept
  {
    context.FillRect(FloatRect(cornerRect), Color::white);
  }

  ScrollbarButtonPressAction ScrollbarTheme::HandleMousePressEvent(Scrollbar &,
                                                                   const PlatformMouseEvent &event,
                                                                   ScrollbarPart pressedPart) noexcept
  {
    if (event.Button() == MouseButton::Right)
    {
      return ScrollbarButtonPressAction::None;
    }
    if (pressedPart == ThumbPart)
    {
      return ScrollbarButtonPressAction::StartDrag;
    }
    return ScrollbarButtonPressAction::Scroll;
  }
}
