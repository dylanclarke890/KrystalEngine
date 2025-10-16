#pragma once

#include "Krystal.UI/Enums/Align.hpp"
#include "Krystal.UI/Node/Node.hpp"

namespace Krys::UI
{
  inline Align ResolveChildAlignment(const Node *node, const Node *child)
  {
    const Align align = child->GetStyle().GetAlignSelf() == Align::Auto ? node->GetStyle().GetAlignItems()
                                                                        : child->GetStyle().GetAlignSelf();
    if (align == Align::Baseline && IsColumn(node->GetStyle().GetFlexDirection()))
    {
      return Align::FlexStart;
    }
    return align;
  }

  /// @brief Fallback alignment to use on overflow
  /// https://www.w3.org/TR/css-align-3/#distribution-values
  constexpr Align FallbackAlignment(Align align)
  {
    switch (align)
    {
        // Fallback to flex-start
      case Align::SpaceBetween:
      case Align::Stretch:      return Align::FlexStart;

      // Fallback to safe center. TODO (T208209388): This should be aligned to
      // Start instead of FlexStart (for row-reverse containers)
      case Align::SpaceAround:
      case Align::SpaceEvenly:  return Align::FlexStart;
      default:                  return align;
    }
  }

  /// @brief Fallback alignment to use on overflow
  /// https://www.w3.org/TR/css-align-3/#distribution-values
  constexpr Justify FallbackAlignment(Justify align)
  {
    switch (align)
    {
        // Fallback to flex-start
      case Justify::SpaceBetween:
        // TODO: Support `justify-content: stretch`
        // case Justify::Stretch:
        return Justify::FlexStart;

      // Fallback to safe center. TODO (T208209388): This should be aligned to
      // Start instead of FlexStart (for row-reverse containers)
      case Justify::SpaceAround:
      case Justify::SpaceEvenly: return Justify::FlexStart;
      default:                   return align;
    }
  }
}