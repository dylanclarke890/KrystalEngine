#include "Krystal.Booey/Graphics/Adwaita/ToggleButtonAdwaita.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"

namespace krys::boo
{
  using namespace adwaita;

  namespace
  {
    // A square as large as fits, centred in the control's rect.
    KRYS_NODISCARD FloatRect ButtonRect(const FloatRoundedRect &borderRect) noexcept
    {
      FloatRect fieldRect = borderRect.Rect();
      if (fieldRect.Width() != fieldRect.Height())
      {
        auto buttonSize = std::min(fieldRect.Width(), fieldRect.Height());
        fieldRect.SetSize({buttonSize, buttonSize});
        if (fieldRect.Width() != borderRect.Rect().Width())
        {
          fieldRect.Move((borderRect.Rect().Width() - fieldRect.Width()) / 2.0f, 0);
        }
        else
        {
          fieldRect.Move(0, (borderRect.Rect().Height() - fieldRect.Height()) / 2.0f);
        }
      }
      return fieldRect;
    }
  }

  ToggleButtonAdwaita::ToggleButtonAdwaita(ControlPart &part, ControlFactoryAdwaita &controlFactory) noexcept
      : ControlAdwaita(part, controlFactory)
  {
  }

  void ToggleButtonAdwaita::Draw(GraphicsContext &graphicsContext, const FloatRoundedRect &borderRect,
                                 float deviceScaleFactor, const ControlStyle &style) noexcept
  {
    if (_owningPart.GetType() == StyleAppearance::Checkbox)
    {
      DrawCheckbox(graphicsContext, borderRect, deviceScaleFactor, style);
    }
    else
    {
      DrawRadio(graphicsContext, borderRect, deviceScaleFactor, style);
    }
  }

  void ToggleButtonAdwaita::DrawCheckbox(GraphicsContext &graphicsContext, const FloatRoundedRect &borderRect,
                                         float /*deviceScaleFactor*/, const ControlStyle &style) noexcept
  {
    GraphicsContextStateSaver stateSaver(graphicsContext);

    FloatRect fieldRect = ButtonRect(borderRect);

    SRGBA<uint8> toggleBorderColor;
    SRGBA<uint8> toggleBorderHoverColor;

    if (style.States.Contains(ControlStyle::State::DarkAppearance))
    {
      toggleBorderColor = ToggleBorderColorDark;
      toggleBorderHoverColor = ToggleBorderHoveredColorDark;
    }
    else
    {
      toggleBorderColor = ToggleBorderColorLight;
      toggleBorderHoverColor = ToggleBorderHoveredColorLight;
    }

    Color accentColor = AccentColor(style);
    Color foregroundColor = accentColor.Luminance() > 0.5 ? Color(SRGBA<uint8> {0, 0, 0, 204}) : Color::white;
    Color accentHoverColor = BlendSourceOver(accentColor, foregroundColor.ColorWithAlphaMultipliedBy(0.1f));

    if (!style.States.Contains(ControlStyle::State::Enabled))
    {
      graphicsContext.BeginTransparencyLayer(static_cast<float>(DisabledOpacity));
    }

    FloatSize corner(2, 2);
    Path path;

    if (style.States.ContainsAny({ControlStyle::State::Checked, ControlStyle::State::Indeterminate}))
    {
      path.AddRoundedRect(fieldRect, corner);
      graphicsContext.SetFillRule(WindRule::NonZero);
      if (style.States.Contains(ControlStyle::State::Hovered)
          && style.States.Contains(ControlStyle::State::Enabled))
      {
        graphicsContext.SetFillColor(accentHoverColor);
      }
      else
      {
        graphicsContext.SetFillColor(accentColor);
      }
      graphicsContext.FillPath(path);
      path.Clear();

      GraphicsContextStateSaver checkedStateSaver(graphicsContext);
      graphicsContext.Translate(fieldRect.X(), fieldRect.Y());
      graphicsContext.Scale(FloatSize(static_cast<float>(fieldRect.Width() / ToggleSize),
                                      static_cast<float>(fieldRect.Height() / ToggleSize)));
      if (style.States.Contains(ControlStyle::State::Indeterminate))
      {
        path.AddRoundedRect(FloatRect(2, 5, 10, 4), corner);
      }
      else
      {
        path.MoveTo({2.43f, 6.57f});
        path.AddLineTo({7.5f, 11.63f});
        path.AddLineTo({14, 5});
        path.AddLineTo({14, 1});
        path.AddLineTo({7.5f, 7.38f});
        path.AddLineTo({4.56f, 4.44f});
        path.CloseSubpath();
      }

      graphicsContext.SetFillColor(foregroundColor);
      graphicsContext.FillPath(path);
    }
    else
    {
      path.AddRoundedRect(fieldRect, corner);
      if (style.States.Contains(ControlStyle::State::Hovered)
          && style.States.Contains(ControlStyle::State::Enabled))
      {
        graphicsContext.SetFillColor(toggleBorderHoverColor);
      }
      else
      {
        graphicsContext.SetFillColor(toggleBorderColor);
      }
      graphicsContext.FillPath(path);
      path.Clear();

      fieldRect.Inflate(-ToggleBorderSize);
      corner.Expand(-ButtonBorderSize, -ButtonBorderSize);
      path.AddRoundedRect(fieldRect, corner);
      graphicsContext.SetFillColor(foregroundColor);
      graphicsContext.FillPath(path);
    }

    if (style.States.Contains(ControlStyle::State::Focused))
    {
      PaintFocus(graphicsContext, borderRect.Rect(), ToggleFocusOffset, FocusColor(accentColor));
    }

    if (!style.States.Contains(ControlStyle::State::Enabled))
    {
      graphicsContext.EndTransparencyLayer();
    }
  }

  void ToggleButtonAdwaita::DrawRadio(GraphicsContext &graphicsContext, const FloatRoundedRect &borderRect,
                                      float /*deviceScaleFactor*/, const ControlStyle &style) noexcept
  {
    GraphicsContextStateSaver stateSaver(graphicsContext);

    FloatRect fieldRect = ButtonRect(borderRect);

    SRGBA<uint8> toggleBorderColor;
    SRGBA<uint8> toggleBorderHoverColor;

    if (style.States.Contains(ControlStyle::State::DarkAppearance))
    {
      toggleBorderColor = ToggleBorderColorDark;
      toggleBorderHoverColor = ToggleBorderHoveredColorDark;
    }
    else
    {
      toggleBorderColor = ToggleBorderColorLight;
      toggleBorderHoverColor = ToggleBorderHoveredColorLight;
    }

    Color accentColor = AccentColor(style);
    Color foregroundColor = accentColor.Luminance() > 0.5 ? Color(SRGBA<uint8> {0, 0, 0, 204}) : Color::white;
    Color accentHoverColor = BlendSourceOver(accentColor, foregroundColor.ColorWithAlphaMultipliedBy(0.1f));

    if (!style.States.Contains(ControlStyle::State::Enabled))
    {
      graphicsContext.BeginTransparencyLayer(static_cast<float>(DisabledOpacity));
    }

    Path path;

    if (style.States.ContainsAny({ControlStyle::State::Checked, ControlStyle::State::Indeterminate}))
    {
      path.AddEllipseInRect(fieldRect);
      graphicsContext.SetFillRule(WindRule::NonZero);
      if (style.States.Contains(ControlStyle::State::Hovered)
          && style.States.Contains(ControlStyle::State::Enabled))
      {
        graphicsContext.SetFillColor(accentHoverColor);
      }
      else
      {
        graphicsContext.SetFillColor(accentColor);
      }
      graphicsContext.FillPath(path);
      path.Clear();

      fieldRect.Inflate(-(fieldRect.Width() - fieldRect.Width() * 0.70f));
      path.AddEllipseInRect(fieldRect);
      graphicsContext.SetFillColor(foregroundColor);
      graphicsContext.FillPath(path);
    }
    else
    {
      path.AddEllipseInRect(fieldRect);
      if (style.States.Contains(ControlStyle::State::Hovered)
          && style.States.Contains(ControlStyle::State::Enabled))
      {
        graphicsContext.SetFillColor(toggleBorderHoverColor);
      }
      else
      {
        graphicsContext.SetFillColor(toggleBorderColor);
      }
      graphicsContext.FillPath(path);
      path.Clear();

      fieldRect.Inflate(-ToggleBorderSize);
      path.AddEllipseInRect(fieldRect);
      graphicsContext.SetFillColor(foregroundColor);
      graphicsContext.FillPath(path);
    }

    if (style.States.Contains(ControlStyle::State::Focused))
    {
      PaintFocus(graphicsContext, borderRect.Rect(), ToggleFocusOffset, FocusColor(accentColor),
                 PaintRounded::Yes);
    }

    if (!style.States.Contains(ControlStyle::State::Enabled))
    {
      graphicsContext.EndTransparencyLayer();
    }
  }
}
