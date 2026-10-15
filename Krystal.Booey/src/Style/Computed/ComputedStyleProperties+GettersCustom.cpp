#include "Krystal.Booey/Style/Computed/ComputedStyleProperties+GettersCustomInlines.hpp"

#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  const boo::Color &ComputedStyleProperties::GetColor() const noexcept
  {
    return _inheritedData->Color;
  }

  LineWidth ComputedStyleProperties::OutlineWidth() const noexcept
  {
    auto &outline = _nonInheritedData->BackgroundData->Outline;
    if (static_cast<OutlineStyle>(outline.OutlineStyle) == OutlineStyle::None)
    {
      return 0_css_px;
    }
    if (static_cast<OutlineStyle>(outline.OutlineStyle) == OutlineStyle::Auto)
    {
      return LineWidth {std::max(Evaluate<float>(outline.OutlineWidth, ZoomNeeded {}),
                                 RenderTheme::PlatformFocusRingWidth())};
    }
    return outline.OutlineWidth;
  }

  Length<> ComputedStyleProperties::OutlineOffset() const noexcept
  {
    auto &outline = _nonInheritedData->BackgroundData->Outline;
    if (static_cast<OutlineStyle>(outline.OutlineStyle) == OutlineStyle::Auto)
    {
      return Length<> {
        Evaluate<float>(outline.OutlineOffset, ZoomNeeded {})
        + RenderTheme::PlatformFocusRingOffset(Evaluate<float>(outline.OutlineWidth, ZoomNeeded {}))};
    }
    return outline.OutlineOffset;
  }
}
