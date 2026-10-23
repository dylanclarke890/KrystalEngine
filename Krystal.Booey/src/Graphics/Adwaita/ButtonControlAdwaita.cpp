#include "Krystal.Booey/Graphics/Adwaita/ButtonControlAdwaita.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"

namespace krys::boo
{
  using namespace adwaita;

  ButtonControlAdwaita::ButtonControlAdwaita(ControlPart &part,
                                             ControlFactoryAdwaita &controlFactory) noexcept
      : ControlAdwaita(part, controlFactory)
  {
  }

  void ButtonControlAdwaita::Draw(GraphicsContext &graphicsContext, const FloatRoundedRect &borderRect,
                                  float deviceScaleFactor, const ControlStyle &style) noexcept
  {
    DrawButton(graphicsContext, borderRect, deviceScaleFactor, style);
  }

  void ButtonControlAdwaita::DrawButton(GraphicsContext &graphicsContext, const FloatRoundedRect &borderRect,
                                        float /*deviceScaleFactor*/, const ControlStyle &style) noexcept
  {
    GraphicsContextStateSaver stateSaver(graphicsContext);

    SRGBA<uint8> buttonBorderColor;
    SRGBA<uint8> buttonBackgroundColor;
    SRGBA<uint8> buttonBackgroundHoveredColor;
    SRGBA<uint8> buttonBackgroundPressedColor;

    if (style.States.Contains(ControlStyle::State::DarkAppearance))
    {
      buttonBorderColor = ButtonBorderColorDark;
      buttonBackgroundColor = ButtonBackgroundColorDark;
      buttonBackgroundHoveredColor = ButtonBackgroundHoveredColorDark;
      buttonBackgroundPressedColor = ButtonBackgroundPressedColorDark;
    }
    else
    {
      buttonBorderColor = ButtonBorderColorLight;
      buttonBackgroundColor = ButtonBackgroundColorLight;
      buttonBackgroundHoveredColor = ButtonBackgroundHoveredColorLight;
      buttonBackgroundPressedColor = ButtonBackgroundPressedColorLight;
    }

    if (!style.States.Contains(ControlStyle::State::Enabled))
    {
      graphicsContext.BeginTransparencyLayer(static_cast<float>(DisabledOpacity));
    }

    FloatRect fieldRect = borderRect.Rect();
    FloatSize corner(5, 5);
    Path path;
    path.AddRoundedRect(fieldRect, corner);
    fieldRect.Inflate(-ButtonBorderSize);
    corner.Expand(-ButtonBorderSize, -ButtonBorderSize);
    path.AddRoundedRect(fieldRect, corner);
    graphicsContext.SetFillRule(WindRule::EvenOdd);
    graphicsContext.SetFillColor(buttonBorderColor);
    graphicsContext.FillPath(path);
    path.Clear();

    path.AddRoundedRect(fieldRect, corner);
    graphicsContext.SetFillRule(WindRule::NonZero);
    if (style.States.Contains(ControlStyle::State::Pressed))
    {
      graphicsContext.SetFillColor(buttonBackgroundPressedColor);
    }
    else if (style.States.Contains(ControlStyle::State::Enabled)
             && style.States.Contains(ControlStyle::State::Hovered))
    {
      graphicsContext.SetFillColor(buttonBackgroundHoveredColor);
    }
    else
    {
      graphicsContext.SetFillColor(buttonBackgroundColor);
    }
    graphicsContext.FillPath(path);

    if (style.States.Contains(ControlStyle::State::Focused))
    {
      PaintFocus(graphicsContext, borderRect.Rect(), ButtonFocusOffset, SystemFocusRingColor());
    }

    if (!style.States.Contains(ControlStyle::State::Enabled))
    {
      graphicsContext.EndTransparencyLayer();
    }
  }
}
