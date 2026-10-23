#include "Krystal.Booey/Graphics/Adwaita/SliderThumbAdwaita.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"

namespace krys::boo
{
  using namespace adwaita;

  SliderThumbAdwaita::SliderThumbAdwaita(ControlPart &part, ControlFactoryAdwaita &controlFactory) noexcept
      : ControlAdwaita(part, controlFactory)
  {
  }

  void SliderThumbAdwaita::Draw(GraphicsContext &graphicsContext, const FloatRoundedRect &borderRect,
                                float /*deviceScaleFactor*/, const ControlStyle &style) noexcept
  {
    GraphicsContextStateSaver stateSaver(graphicsContext);

    SRGBA<uint8> sliderThumbBackgroundColor;
    SRGBA<uint8> sliderThumbBackgroundHoveredColor;
    SRGBA<uint8> sliderThumbBackgroundDisabledColor;
    SRGBA<uint8> sliderThumbBorderColor;

    if (style.States.Contains(ControlStyle::State::DarkAppearance))
    {
      sliderThumbBackgroundColor = SliderThumbBackgroundColorDark;
      sliderThumbBackgroundHoveredColor = SliderThumbBackgroundHoveredColorDark;
      sliderThumbBackgroundDisabledColor = SliderThumbBackgroundDisabledColorDark;
      sliderThumbBorderColor = SliderThumbBorderColorDark;
    }
    else
    {
      sliderThumbBackgroundColor = SliderThumbBackgroundColorLight;
      sliderThumbBackgroundHoveredColor = SliderThumbBackgroundHoveredColorLight;
      sliderThumbBackgroundDisabledColor = SliderThumbBackgroundDisabledColorLight;
      sliderThumbBorderColor = SliderThumbBorderColorLight;
    }

    FloatRect fieldRect = borderRect.Rect();
    Path path;
    path.AddEllipseInRect(fieldRect);
    fieldRect.Inflate(-SliderThumbBorderSize);
    path.AddEllipseInRect(fieldRect);
    graphicsContext.SetFillRule(WindRule::EvenOdd);
    if (style.States.Contains(ControlStyle::State::Enabled)
        && style.States.Contains(ControlStyle::State::Pressed))
    {
      graphicsContext.SetFillColor(AccentColor(style));
    }
    else
    {
      graphicsContext.SetFillColor(sliderThumbBorderColor);
    }
    graphicsContext.FillPath(path);
    path.Clear();

    path.AddEllipseInRect(fieldRect);
    graphicsContext.SetFillRule(WindRule::NonZero);
    if (!style.States.Contains(ControlStyle::State::Enabled))
    {
      graphicsContext.SetFillColor(sliderThumbBackgroundDisabledColor);
    }
    else if (style.States.Contains(ControlStyle::State::Hovered))
    {
      graphicsContext.SetFillColor(sliderThumbBackgroundHoveredColor);
    }
    else
    {
      graphicsContext.SetFillColor(sliderThumbBackgroundColor);
    }
    graphicsContext.FillPath(path);
  }
}
