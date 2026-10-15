#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/CSS2/ZIndex.hpp"
#include "Krystal.Booey/Style/Values/Inline/VerticalAlign.hpp"
#include "Krystal.Booey/Style/Values/Sizing/MaximumSize.hpp"
#include "Krystal.Booey/Style/Values/Sizing/MinimumSize.hpp"
#include "Krystal.Booey/Style/Values/Sizing/PreferredSize.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleBoxData : public RefCounted<StyleBoxData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleBoxData> Create() noexcept
    {
      return AdoptRef(*new StyleBoxData);
    }
    KRYS_NODISCARD Ref<StyleBoxData> Copy() const noexcept;

    bool operator==(const StyleBoxData &) const noexcept;

    style::PreferredSize Width;
    style::PreferredSize Height;

    style::MinimumSize MinWidth;
    style::MinimumSize MinHeight;

    style::MaximumSize MaxWidth;
    style::MaximumSize MaxHeight;

    style::VerticalAlign VerticalAlign;

    uint8 HasAutoSpecifiedZIndex : 1;
    uint8 HasAutoUsedZIndex : 1;
    uint8 BoxSizing : 1;
    uint8 BoxDecorationBreak : 1;

    style::ZIndex::Value SpecifiedZIndexValue;
    style::ZIndex::Value UsedZIndexValue;

    KRYS_NODISCARD style::ZIndex SpecifiedZIndex() const noexcept
    {
      return {static_cast<bool>(HasAutoSpecifiedZIndex), SpecifiedZIndexValue};
    }
    KRYS_NODISCARD style::ZIndex UsedZIndex() const noexcept
    {
      return {static_cast<bool>(HasAutoUsedZIndex), UsedZIndexValue};
    }

  private:
    StyleBoxData() noexcept;
    StyleBoxData(const StyleBoxData &) noexcept;
  };
}
