/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Graham Dennis (graham.dennis@gmail.com)
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

    unsigned ColumnFill : 1;
    unsigned ColumnSpan : 1;

  private:
    StyleMultiColData() noexcept;
    StyleMultiColData(const StyleMultiColData &) noexcept;
  };
}
