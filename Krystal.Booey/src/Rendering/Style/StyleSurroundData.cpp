#include "Krystal.Booey/Rendering/Style/StyleSurroundData.hpp"

#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo
{
  using namespace css::Literals;

  StyleSurroundData::StyleSurroundData() noexcept
      : HasExplicitlySetBorderBottomLeftRadius(false), HasExplicitlySetBorderBottomRightRadius(false),
        HasExplicitlySetBorderTopLeftRadius(false), HasExplicitlySetBorderTopRightRadius(false),
        HasExplicitlySetPaddingBottom(false), HasExplicitlySetPaddingLeft(false),
        HasExplicitlySetPaddingRight(false), HasExplicitlySetPaddingTop(false), Inset(css::keywords::Auto {}),
        Margin(0_css_px), Padding(0_css_px)
  {
  }

  inline StyleSurroundData::StyleSurroundData(const StyleSurroundData &o) noexcept
      : RefCounted<StyleSurroundData>(),
        HasExplicitlySetBorderBottomLeftRadius(o.HasExplicitlySetBorderBottomLeftRadius),
        HasExplicitlySetBorderBottomRightRadius(o.HasExplicitlySetBorderBottomRightRadius),
        HasExplicitlySetBorderTopLeftRadius(o.HasExplicitlySetBorderTopLeftRadius),
        HasExplicitlySetBorderTopRightRadius(o.HasExplicitlySetBorderTopRightRadius),
        HasExplicitlySetPaddingBottom(o.HasExplicitlySetPaddingBottom),
        HasExplicitlySetPaddingLeft(o.HasExplicitlySetPaddingLeft),
        HasExplicitlySetPaddingRight(o.HasExplicitlySetPaddingRight),
        HasExplicitlySetPaddingTop(o.HasExplicitlySetPaddingTop), Inset(o.Inset), Margin(o.Margin),
        Padding(o.Padding), Border(o.Border)
  {
  }

  Ref<StyleSurroundData> StyleSurroundData::Copy() const noexcept
  {
    return AdoptRef(*new StyleSurroundData(*this));
  }

  bool StyleSurroundData::operator==(const StyleSurroundData &o) const noexcept
  {
    return Inset == o.Inset && Margin == o.Margin && Padding == o.Padding && Border == o.Border
           && HasExplicitlySetBorderBottomLeftRadius == o.HasExplicitlySetBorderBottomLeftRadius
           && HasExplicitlySetBorderBottomRightRadius == o.HasExplicitlySetBorderBottomRightRadius
           && HasExplicitlySetBorderTopLeftRadius == o.HasExplicitlySetBorderTopLeftRadius
           && HasExplicitlySetBorderTopRightRadius == o.HasExplicitlySetBorderTopRightRadius
           && HasExplicitlySetPaddingBottom == o.HasExplicitlySetPaddingBottom
           && HasExplicitlySetPaddingLeft == o.HasExplicitlySetPaddingLeft
           && HasExplicitlySetPaddingRight == o.HasExplicitlySetPaddingRight
           && HasExplicitlySetPaddingTop == o.HasExplicitlySetPaddingTop;
  }
}
