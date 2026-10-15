#include "Krystal.Booey/Rendering/Style/StyleMultiColData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  StyleMultiColData::StyleMultiColData() noexcept
      : ColumnWidth(style::ComputedStyle::InitialColumnWidth()),
        ColumnCount(style::ComputedStyle::InitialColumnCount()),
        ColumnFill(static_cast<unsigned>(style::ComputedStyle::InitialColumnFill())),
        ColumnSpan(static_cast<unsigned>(style::ComputedStyle::InitialColumnSpan())),
        ColumnProgression(static_cast<unsigned>(style::ComputedStyle::InitialColumnProgression()))
  {
  }

  inline StyleMultiColData::StyleMultiColData(const StyleMultiColData &other) noexcept
      : RefCounted<StyleMultiColData>(), ColumnWidth(other.ColumnWidth), ColumnCount(other.ColumnCount),
        ColumnRule(other.ColumnRule), VisitedLinkColumnRuleColor(other.VisitedLinkColumnRuleColor),
        ColumnFill(other.ColumnFill), ColumnSpan(other.ColumnSpan), ColumnProgression(other.ColumnProgression)
  {
  }

  Ref<StyleMultiColData> StyleMultiColData::Copy() const noexcept
  {
    return AdoptRef(*new StyleMultiColData(*this));
  }

  bool StyleMultiColData::operator==(const StyleMultiColData &other) const noexcept
  {
    return ColumnWidth == other.ColumnWidth && ColumnCount == other.ColumnCount
           && ColumnRule == other.ColumnRule && VisitedLinkColumnRuleColor == other.VisitedLinkColumnRuleColor
           && ColumnFill == other.ColumnFill && ColumnSpan == other.ColumnSpan
           && ColumnProgression == other.ColumnProgression;
  }

  style::LineWidth StyleMultiColData::ColumnRuleWidth() const noexcept
  {
    if (!ColumnRule.HasVisibleStyle())
    {
      return style::LineWidth {0_css_px};
    }
    return ColumnRule.Width;
  }
}
