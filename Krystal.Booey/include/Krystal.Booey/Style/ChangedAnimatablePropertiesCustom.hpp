#pragma once

#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/Rendering/Style/StyleBoxData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleRareInheritedData.hpp"
#include "Krystal.Booey/Style/ChangedAnimatablePropertiesGenerated.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+GettersInlines.hpp"

namespace krys::boo::style
{
  /// @brief The changed animatable properties whose storage the generated collection can't compare.
  class ChangedAnimatablePropertiesCustom final
  {
  public:
    static void ConservativelyCollectChangedAnimatablePropertiesForCursor(
      const ComputedStyle::InheritedFlags &a, const ComputedStyle::InheritedFlags &b,
      CSSPropertiesBitSet &changingProperties) noexcept
    {
      if (a.CursorType != b.CursorType)
      {
        changingProperties.Set(css::PropertyId::Cursor);
      }
    }

    static void ConservativelyCollectChangedAnimatablePropertiesForZIndex(
      const StyleBoxData &a, const StyleBoxData &b, CSSPropertiesBitSet &changingProperties) noexcept
    {
      if (a.SpecifiedZIndex() != b.SpecifiedZIndex())
      {
        changingProperties.Set(css::PropertyId::ZIndex);
      }
    }

    static void ConservativelyCollectChangedAnimatablePropertiesForCaretColor(
      const StyleRareInheritedData &a, const StyleRareInheritedData &b,
      CSSPropertiesBitSet &changingProperties) noexcept
    {
      if (a.CaretColor != b.CaretColor || a.VisitedLinkCaretColor != b.VisitedLinkCaretColor
          || a.HasAutoCaretColor != b.HasAutoCaretColor
          || a.HasVisitedLinkAutoCaretColor != b.HasVisitedLinkAutoCaretColor)
      {
        changingProperties.Set(css::PropertyId::CaretColor);
      }
    }
  };
}
