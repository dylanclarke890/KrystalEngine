#pragma once

#include "Krystal.Booey/Rendering/TableLayout.hpp"
#include "Krystal.Booey/Style/Values/Sizing/PreferredSize.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  /// @brief The fixed table layout algorithm: column widths come from the columns and the first row only.
  // https://www.w3.org/TR/CSS22/tables.html#fixed-table-layout
  class FixedTableLayout final : public TableLayout
  {
  private:
    List<style::PreferredSize> _width;

  public:
    explicit FixedTableLayout(RenderTable *table) noexcept;

    void ComputeIntrinsicLogicalWidths(LayoutUnit &minWidth, LayoutUnit &maxWidth,
                                       TableIntrinsics intrinsics) noexcept override;
    void ApplyPreferredLogicalWidthQuirks(LayoutUnit &minWidth, LayoutUnit &maxWidth) const noexcept override;
    void Layout() noexcept override;

  private:
    KRYS_NODISCARD float CalcWidthArray() noexcept;
  };
}
