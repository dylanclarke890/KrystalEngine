/*
 * Copyright (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004-2013 Apple Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
 *
 */

#include "Krystal.Booey/Rendering/Style/StyleMultiColData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  StyleMultiColData::StyleMultiColData() noexcept
      : ColumnWidth(style::ComputedStyle::InitialColumnWidth()),
        ColumnCount(style::ComputedStyle::InitialColumnCount()),
        ColumnFill(static_cast<unsigned>(style::ComputedStyle::InitialColumnFill())),
        ColumnSpan(static_cast<unsigned>(style::ComputedStyle::InitialColumnSpan()))
  {
  }

  inline StyleMultiColData::StyleMultiColData(const StyleMultiColData &other) noexcept
      : RefCounted<StyleMultiColData>(), ColumnWidth(other.ColumnWidth), ColumnCount(other.ColumnCount),
        ColumnRule(other.ColumnRule), ColumnFill(other.ColumnFill), ColumnSpan(other.ColumnSpan)
  {
  }

  Ref<StyleMultiColData> StyleMultiColData::Copy() const noexcept
  {
    return AdoptRef(*new StyleMultiColData(*this));
  }

  bool StyleMultiColData::operator==(const StyleMultiColData &other) const noexcept
  {
    return ColumnWidth == other.ColumnWidth && ColumnCount == other.ColumnCount
           && ColumnRule == other.ColumnRule && ColumnFill == other.ColumnFill
           && ColumnSpan == other.ColumnSpan;
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
