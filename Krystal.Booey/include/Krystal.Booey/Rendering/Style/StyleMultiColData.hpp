#pragma once

#include "Krystal.Booey/Rendering/Style/BorderValue.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/LineWidth.hpp"
#include "Krystal.Booey/Style/Values/Multicol/ColumnCount.hpp"
#include "Krystal.Booey/Style/Values/Multicol/ColumnWidth.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo
{
  // CSS3 Multi Column Layout
  class StyleMultiColData : public RefCounted<StyleMultiColData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleMultiColData> Create() noexcept
    {
      return AdoptRef(*new StyleMultiColData);
    }
    KRYS_NODISCARD Ref<StyleMultiColData> Copy() const noexcept;

    bool operator==(const StyleMultiColData &) const noexcept;

    KRYS_NODISCARD style::LineWidth ColumnRuleWidth() const noexcept;

    style::ColumnWidth ColumnWidth {css::keywords::Auto {}};
    style::ColumnCount ColumnCount {css::keywords::Auto {}};
    BorderValue ColumnRule;
    style::Color VisitedLinkColumnRuleColor;

    unsigned ColumnFill : 1;
    unsigned ColumnSpan : 1;
    unsigned ColumnProgression : 2;

  private:
    StyleMultiColData() noexcept;
    StyleMultiColData(const StyleMultiColData &) noexcept;
  };
}
