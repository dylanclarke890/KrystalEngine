/*
 * Copyright (C) 2002 Lars Knoll (knoll@kde.org)
 *           (C) 2002 Dirk Mueller (mueller@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/Rendering/Tables/TableLayout.hpp"
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
