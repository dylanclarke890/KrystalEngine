#include "Krystal.Booey/Graphics/Adwaita/InnerSpinButtonAdwaita.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"

namespace krys::boo
{
  using namespace adwaita;

  InnerSpinButtonAdwaita::InnerSpinButtonAdwaita(ControlPart &part,
                                                 ControlFactoryAdwaita &controlFactory) noexcept
      : ControlAdwaita(part, controlFactory)
  {
  }

  void InnerSpinButtonAdwaita::Draw(GraphicsContext &graphicsContext, const FloatRoundedRect &borderRect,
                                    float /*deviceScaleFactor*/, const ControlStyle &style) noexcept
  {
    GraphicsContextStateSaver stateSaver(graphicsContext);

    SRGBA<uint8> spinButtonBorderColor;
    SRGBA<uint8> spinButtonBackgroundColor;
    SRGBA<uint8> spinButtonBackgroundHoveredColor;
    SRGBA<uint8> spinButtonBackgroundPressedColor;

    bool useDarkAppearance = style.States.Contains(ControlStyle::State::DarkAppearance);
    if (useDarkAppearance)
    {
      spinButtonBorderColor = SpinButtonBorderColorDark;
      spinButtonBackgroundColor = SpinButtonBackgroundColorDark;
      spinButtonBackgroundHoveredColor = SpinButtonBackgroundHoveredColorDark;
      spinButtonBackgroundPressedColor = SpinButtonBackgroundPressedColorDark;
    }
    else
    {
      spinButtonBorderColor = SpinButtonBorderColorLight;
      spinButtonBackgroundColor = SpinButtonBackgroundColorLight;
      spinButtonBackgroundHoveredColor = SpinButtonBackgroundHoveredColorLight;
      spinButtonBackgroundPressedColor = SpinButtonBackgroundPressedColorLight;
    }

    FloatRect fieldRect = borderRect.Rect();
    FloatSize corner(2, 2);
    Path path;
    path.AddRoundedRect(fieldRect, corner);
    fieldRect.Inflate(-ButtonBorderSize);
    corner.Expand(-ButtonBorderSize, -ButtonBorderSize);
    path.AddRoundedRect(fieldRect, corner);
    graphicsContext.SetFillRule(WindRule::EvenOdd);
    graphicsContext.SetFillColor(spinButtonBorderColor);
    graphicsContext.FillPath(path);
    path.Clear();

    path.AddRoundedRect(fieldRect, corner);
    graphicsContext.SetFillRule(WindRule::NonZero);
    graphicsContext.SetFillColor(spinButtonBackgroundColor);
    graphicsContext.FillPath(path);
    path.Clear();

    FloatRect buttonRect = fieldRect;
    buttonRect.SetHeight(fieldRect.Height() / 2.0f);
    {
      if (style.States.Contains(ControlStyle::State::SpinUp))
      {
        path.AddRoundedRect(FloatRoundedRect(buttonRect, corner, corner, {}, {}));
        if (style.States.Contains(ControlStyle::State::Pressed))
        {
          graphicsContext.SetFillColor(spinButtonBackgroundPressedColor);
        }
        else if (style.States.Contains(ControlStyle::State::Hovered))
        {
          graphicsContext.SetFillColor(spinButtonBackgroundHoveredColor);
        }
        graphicsContext.FillPath(path);
        path.Clear();
      }

      PaintArrow(graphicsContext, buttonRect, ArrowDirection::Up, useDarkAppearance);
    }

    buttonRect.Move(0, buttonRect.Height());
    {
      if (!style.States.Contains(ControlStyle::State::SpinUp))
      {
        path.AddRoundedRect(FloatRoundedRect(buttonRect, {}, {}, corner, corner));
        if (style.States.Contains(ControlStyle::State::Pressed))
        {
          graphicsContext.SetFillColor(spinButtonBackgroundPressedColor);
        }
        else if (style.States.Contains(ControlStyle::State::Hovered))
        {
          graphicsContext.SetFillColor(spinButtonBackgroundHoveredColor);
        }
        else
        {
          graphicsContext.SetFillColor(spinButtonBackgroundColor);
        }
        graphicsContext.FillPath(path);
        path.Clear();
      }

      PaintArrow(graphicsContext, buttonRect, ArrowDirection::Down, useDarkAppearance);
    }
  }
}
