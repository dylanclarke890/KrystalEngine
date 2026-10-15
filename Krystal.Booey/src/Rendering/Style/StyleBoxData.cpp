#include "Krystal.Booey/Rendering/Style/StyleBoxData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  StyleBoxData::StyleBoxData() noexcept
      : Width(style::ComputedStyle::InitialWidth()), Height(style::ComputedStyle::InitialHeight()),
        MinWidth(style::ComputedStyle::InitialMinWidth()),
        MinHeight(style::ComputedStyle::InitialMinHeight()),
        MaxWidth(style::ComputedStyle::InitialMaxWidth()),
        MaxHeight(style::ComputedStyle::InitialMaxHeight()),
        VerticalAlign(style::ComputedStyle::InitialVerticalAlign()),
        HasAutoSpecifiedZIndex(static_cast<uint8>(style::ComputedStyle::InitialSpecifiedZIndex()._isAuto)),
        HasAutoUsedZIndex(static_cast<uint8>(style::ComputedStyle::InitialUsedZIndex()._isAuto)),
        BoxSizing(static_cast<uint8>(boo::BoxSizing::ContentBox)),
        BoxDecorationBreak(static_cast<uint8>(boo::BoxDecorationBreak::Slice)),
        SpecifiedZIndexValue(style::ComputedStyle::InitialSpecifiedZIndex()._value),
        UsedZIndexValue(style::ComputedStyle::InitialUsedZIndex()._value)
  {
  }

  inline StyleBoxData::StyleBoxData(const StyleBoxData &o) noexcept
      : RefCounted<StyleBoxData>(), Width(o.Width), Height(o.Height), MinWidth(o.MinWidth),
        MinHeight(o.MinHeight), MaxWidth(o.MaxWidth), MaxHeight(o.MaxHeight), VerticalAlign(o.VerticalAlign),
        HasAutoSpecifiedZIndex(o.HasAutoSpecifiedZIndex), HasAutoUsedZIndex(o.HasAutoUsedZIndex),
        BoxSizing(o.BoxSizing), BoxDecorationBreak(o.BoxDecorationBreak),
        SpecifiedZIndexValue(o.SpecifiedZIndexValue), UsedZIndexValue(o.UsedZIndexValue)
  {
  }

  Ref<StyleBoxData> StyleBoxData::Copy() const noexcept
  {
    return AdoptRef(*new StyleBoxData(*this));
  }

  bool StyleBoxData::operator==(const StyleBoxData &o) const noexcept
  {
    return Width == o.Width && Height == o.Height && MinWidth == o.MinWidth && MinHeight == o.MinHeight
           && MaxWidth == o.MaxWidth && MaxHeight == o.MaxHeight && VerticalAlign == o.VerticalAlign
           && UsedZIndexValue == o.UsedZIndexValue && HasAutoUsedZIndex == o.HasAutoUsedZIndex
           && BoxSizing == o.BoxSizing && BoxDecorationBreak == o.BoxDecorationBreak
           && SpecifiedZIndexValue == o.SpecifiedZIndexValue
           && HasAutoSpecifiedZIndex == o.HasAutoSpecifiedZIndex;
  }
}
