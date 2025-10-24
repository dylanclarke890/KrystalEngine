#pragma once

#include "Krystal.UI.Styles/Enums/Align.hpp"
#include "Krystal.UI.Layout/Node/Node.hpp"

namespace Krys::UI::Layout
{
  inline Styles::Align ResolveChildAlignment(const Node *node, const Node *child)
  {
    const Styles::Align align = child->GetStyle().GetAlignSelf() == Styles::Align::Auto
                                  ? node->GetStyle().GetAlignItems()
                                  : child->GetStyle().GetAlignSelf();
    if (align == Styles::Align::Baseline && IsColumn(node->GetStyle().GetFlexDirection()))
    {
      return Styles::Align::FlexStart;
    }
    return align;
  }

  /// @brief Fallback alignment to use on overflow
  /// https://www.w3.org/TR/css-align-3/#distribution-values
  constexpr Styles::Align FallbackAlignment(Styles::Align align)
  {
    switch (align)
    {
        // Fallback to flex-start
      case Styles::Align::SpaceBetween:
      case Styles::Align::Stretch:      return Styles::Align::FlexStart;

      // Fallback to safe center. TODO (T208209388): This should be aligned to
      // Start instead of FlexStart (for row-reverse containers)
      case Styles::Align::SpaceAround:
      case Styles::Align::SpaceEvenly:  return Styles::Align::FlexStart;
      default:                          return align;
    }
  }

  /// @brief Fallback alignment to use on overflow
  /// https://www.w3.org/TR/css-align-3/#distribution-values
  constexpr Styles::Justify FallbackAlignment(Styles::Justify align)
  {
    switch (align)
    {
        // Fallback to flex-start
      case Styles::Justify::SpaceBetween:
        // TODO: Support `justify-content: stretch`
        // case Justify::Stretch:
        return Styles::Justify::FlexStart;

      // Fallback to safe center. TODO (T208209388): This should be aligned to
      // Start instead of FlexStart (for row-reverse containers)
      case Styles::Justify::SpaceAround:
      case Styles::Justify::SpaceEvenly: return Styles::Justify::FlexStart;
      default:                           return align;
    }
  }
}