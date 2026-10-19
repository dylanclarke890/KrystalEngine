#include "Krystal.Booey/Rendering/TextDecorationPainter.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/HTMLAnchorElement.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  bool TextDecorationPainter::Styles::operator==(const Styles &other) const noexcept
  {
    return Underline.DecorationColor == other.Underline.DecorationColor
           && Overline.DecorationColor == other.Overline.DecorationColor
           && Linethrough.DecorationColor == other.Linethrough.DecorationColor
           && Underline.DecorationStyle == other.Underline.DecorationStyle
           && Overline.DecorationStyle == other.Overline.DecorationStyle
           && Linethrough.DecorationStyle == other.Linethrough.DecorationStyle;
  }

  KRYS_NODISCARD static bool IsAnchor(const dom::Node *node) noexcept
  {
    auto *element = DynamicDowncast<html::HTMLElement>(node);
    return element && Is<html::HTMLAnchorElement>(*element);
  }

  static void CollectStylesForRenderer(TextDecorationPainter::Styles &result, const RenderObject &renderer,
                                       style::TextDecorationLine remainingDecorations, bool firstLineStyle,
                                       EnumSet<PaintBehavior> paintBehavior) noexcept
  {
    using Flag = style::TextDecorationLine::Flag;

    auto extractDecorations = [&](const RenderStyle &style, style::TextDecorationLine decorations)
    {
      if (!decorations.ContainsAny({Flag::Underline, Flag::Overline, Flag::LineThrough}))
      {
        return;
      }

      auto color = TextDecorationPainter::DecorationColor(style, paintBehavior);
      auto decorationStyle = style.GetTextDecorationStyle();

      if (decorations.HasUnderline())
      {
        remainingDecorations.Remove(Flag::Underline);
        result.Underline.DecorationColor = color;
        result.Underline.DecorationStyle = decorationStyle;
      }
      if (decorations.HasOverline())
      {
        remainingDecorations.Remove(Flag::Overline);
        result.Overline.DecorationColor = color;
        result.Overline.DecorationStyle = decorationStyle;
      }
      if (decorations.HasLineThrough())
      {
        remainingDecorations.Remove(Flag::LineThrough);
        result.Linethrough.DecorationColor = color;
        result.Linethrough.DecorationStyle = decorationStyle;
      }
    };

    auto styleForRenderer = [&](const RenderObject &renderer) -> const RenderStyle &
    {
      return firstLineStyle ? renderer.FirstLineStyle() : renderer.Style();
    };

    const RenderObject *current = &renderer;
    do
    {
      const auto &style = styleForRenderer(*current);
      extractDecorations(style, style.GetTextDecorationLine());

      if (current->Style().Display() == DisplayType::RubyAnnotation)
      {
        return;
      }

      current = current->Parent();
      // FIXME: Anonymous blocks of inline continuations carry on at the continuation, once there are
      // continuations.

      if (remainingDecorations.IsNone())
      {
        break;
      }

    } while (current && !IsAnchor(current->GetNode()));

    // If we bailed out, use the element we bailed out at (typically a <font> or <a> element).
    if (!remainingDecorations.IsNone() && current)
    {
      extractDecorations(styleForRenderer(*current), remainingDecorations);
    }
  }

  Color TextDecorationPainter::DecorationColor(const RenderStyle &style,
                                               EnumSet<PaintBehavior> paintBehavior) noexcept
  {
    if (paintBehavior.Contains(PaintBehavior::ForceBlackText))
    {
      return Color::black;
    }

    if (paintBehavior.Contains(PaintBehavior::ForceWhiteText))
    {
      return Color::white;
    }

    return style.VisitedDependentColorWithColorFilter(css::PropertyId::TextDecorationColor, paintBehavior);
  }

  TextDecorationPainter::Styles TextDecorationPainter::StylesForRenderer(
    const RenderObject &renderer, style::TextDecorationLine requestedDecorations, bool firstLineStyle,
    EnumSet<PaintBehavior> paintBehavior) noexcept
  {
    if (requestedDecorations.IsNone())
    {
      return {};
    }

    Styles result;
    CollectStylesForRenderer(result, renderer, requestedDecorations, false, paintBehavior);
    if (firstLineStyle)
    {
      CollectStylesForRenderer(result, renderer, requestedDecorations, true, paintBehavior);
    }
    return result;
  }

  style::TextDecorationLine TextDecorationPainter::TextDecorationsInEffectForStyle(
    const TextDecorationPainter::Styles &style) noexcept
  {
    using Flag = style::TextDecorationLine::Flag;

    EnumSet<Flag> decorations;
    if (style.Underline.DecorationColor.IsValid())
    {
      decorations.Add(Flag::Underline);
    }
    if (style.Overline.DecorationColor.IsValid())
    {
      decorations.Add(Flag::Overline);
    }
    if (style.Linethrough.DecorationColor.IsValid())
    {
      decorations.Add(Flag::LineThrough);
    }
    return decorations;
  }
}
