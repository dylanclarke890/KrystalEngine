#include "Krystal.Booey/Graphics/Adwaita/MenuListAdwaita.hpp"
#include "Krystal.Booey/Graphics/Adwaita/ButtonControlAdwaita.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"

namespace krys::boo
{
  using namespace adwaita;

  MenuListAdwaita::MenuListAdwaita(ControlPart &part, ControlFactoryAdwaita &controlFactory) noexcept
      : ControlAdwaita(part, controlFactory)
  {
  }

  void MenuListAdwaita::Draw(GraphicsContext &graphicsContext, const FloatRoundedRect &borderRect,
                             float deviceScaleFactor, const ControlStyle &style) noexcept
  {
    GraphicsContextStateSaver stateSaver(graphicsContext);

    ButtonControlAdwaita::DrawButton(graphicsContext, borderRect, deviceScaleFactor, style);

    auto zoomedArrowSize = MenuListButtonArrowSize * style.ZoomFactor;
    FloatRect fieldRect = borderRect.Rect();
    auto borderWidth = fieldRect.Width();
    auto borderHeight = fieldRect.Height();
    fieldRect.SetSize({zoomedArrowSize, zoomedArrowSize});
    if (!style.States.Contains(ControlStyle::State::VerticalWritingMode))
    {
      auto centerY = (borderHeight / 2) - (zoomedArrowSize / 2);
      if (style.States.Contains(ControlStyle::State::InlineFlippedWritingMode))
      {
        fieldRect.Move(MenuListButtonPadding, centerY);
      }
      else
      {
        fieldRect.Move(borderWidth - (zoomedArrowSize + MenuListButtonPadding), centerY);
      }
    }
    else
    {
      auto centerX = (borderWidth / 2) - (zoomedArrowSize / 2);
      fieldRect.Move(centerX, borderHeight - (zoomedArrowSize + MenuListButtonPadding));
    }
    PaintArrow(graphicsContext, fieldRect, ArrowDirection::Down,
               style.States.Contains(ControlStyle::State::DarkAppearance));

    if (style.States.Contains(ControlStyle::State::Focused))
    {
      PaintFocus(graphicsContext, borderRect.Rect(), MenuListButtonFocusOffset, SystemFocusRingColor());
    }
  }
}
