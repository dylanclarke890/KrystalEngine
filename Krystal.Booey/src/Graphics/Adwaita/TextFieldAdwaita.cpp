#include "Krystal.Booey/Graphics/Adwaita/TextFieldAdwaita.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"

namespace krys::boo
{
  using namespace adwaita;

  TextFieldAdwaita::TextFieldAdwaita(ControlPart &part, ControlFactoryAdwaita &controlFactory) noexcept
      : ControlAdwaita(part, controlFactory)
  {
  }

  void TextFieldAdwaita::Draw(GraphicsContext &graphicsContext, const FloatRoundedRect &borderRect,
                              float /*deviceScaleFactor*/, const ControlStyle &style) noexcept
  {
    GraphicsContextStateSaver stateSaver(graphicsContext);

    SRGBA<uint8> textFieldBackgroundColor;
    SRGBA<uint8> textFieldBorderColor;

    if (style.States.Contains(ControlStyle::State::DarkAppearance))
    {
      textFieldBackgroundColor = TextFieldBackgroundColorDark;
      textFieldBorderColor = TextFieldBorderColorDark;
    }
    else
    {
      textFieldBackgroundColor = TextFieldBackgroundColorLight;
      textFieldBorderColor = TextFieldBorderColorLight;
    }

    bool enabled = style.States.Contains(ControlStyle::State::Enabled)
                   && !style.States.Contains(ControlStyle::State::ReadOnly);
    int32 borderSize = TextFieldBorderSize;
    if (style.States.Contains(ControlStyle::State::Focused))
    {
      borderSize *= 2;
    }

    if (!enabled)
    {
      graphicsContext.BeginTransparencyLayer(static_cast<float>(DisabledOpacity));
    }

    FloatRect fieldRect = borderRect.Rect();
    FloatSize corner(5, 5);
    Path path;
    path.AddRoundedRect(fieldRect, corner);
    fieldRect.Inflate(static_cast<float>(-borderSize));
    corner.Expand(static_cast<float>(-borderSize), static_cast<float>(-borderSize));
    path.AddRoundedRect(fieldRect, corner);
    graphicsContext.SetFillRule(WindRule::EvenOdd);
    if (enabled && style.States.Contains(ControlStyle::State::Focused))
    {
      graphicsContext.SetFillColor(SystemFocusRingColor());
    }
    else
    {
      graphicsContext.SetFillColor(textFieldBorderColor);
    }
    graphicsContext.FillPath(path);
    path.Clear();

    path.AddRoundedRect(fieldRect, corner);
    graphicsContext.SetFillRule(WindRule::NonZero);
    graphicsContext.SetFillColor(textFieldBackgroundColor);
    graphicsContext.FillPath(path);

    if (style.States.Contains(ControlStyle::State::ListButton))
    {
      auto zoomedArrowSize = MenuListButtonArrowSize * style.ZoomFactor;
      FloatRect arrowRect = borderRect.Rect();
      auto borderWidth = arrowRect.Width();
      auto borderHeight = arrowRect.Height();
      arrowRect.SetSize({zoomedArrowSize, zoomedArrowSize});
      if (!style.States.Contains(ControlStyle::State::VerticalWritingMode))
      {
        auto centerY = (borderHeight / 2) - (zoomedArrowSize / 2);
        if (style.States.Contains(ControlStyle::State::InlineFlippedWritingMode))
        {
          arrowRect.Move(TextFieldBorderSize, centerY);
        }
        else
        {
          arrowRect.Move(borderWidth - zoomedArrowSize + TextFieldBorderSize, centerY);
        }
      }
      else
      {
        auto centerX = (borderWidth / 2) - (zoomedArrowSize / 2);
        arrowRect.Move(centerX, borderHeight - zoomedArrowSize + TextFieldBorderSize);
      }
      bool useDarkAppearance = style.States.Contains(ControlStyle::State::DarkAppearance);
      PaintArrow(graphicsContext, arrowRect, ArrowDirection::Down, useDarkAppearance);
    }

    if (!enabled)
    {
      graphicsContext.EndTransparencyLayer();
    }
  }
}
