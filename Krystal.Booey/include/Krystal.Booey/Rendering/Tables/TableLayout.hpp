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

#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo
{
  class RenderTable;

  /// @brief Which intrinsic widths of a table are wanted: the ones used for layout, or the ones for an
  /// intrinsic sizing keyword.
  enum class TableIntrinsics : uint8
  {
    ForLayout,
    ForKeyword
  };

  /// @brief The algorithm that computes a table's intrinsic widths and the positions of its columns.
  class TableLayout
  {
    KRYS_NON_COPYABLE(TableLayout);

  protected:
    // FIXME: Once we enable SATURATED_LAYOUT_ARITHMETHIC, this should just be LayoutUnit::nearlyMax().
    // Until then though, using nearlyMax causes overflow in some tests, so we just pick a large number.
    constexpr static int32 TableMaxWidth = 1'000'000;

    RenderTable *_table;

  public:
    explicit TableLayout(RenderTable *table) noexcept : _table(table)
    {
    }
    virtual ~TableLayout() noexcept = default;

    virtual void ComputeIntrinsicLogicalWidths(LayoutUnit &minWidth, LayoutUnit &maxWidth,
                                               TableIntrinsics intrinsics) noexcept = 0;
    KRYS_NODISCARD virtual LayoutUnit ScaledWidthFromPercentColumns() const noexcept
    {
      return 0_lu;
    }
    virtual void ApplyPreferredLogicalWidthQuirks(LayoutUnit &minWidth,
                                                  LayoutUnit &maxWidth) const noexcept = 0;
    virtual void Layout() noexcept = 0;
  };
}
