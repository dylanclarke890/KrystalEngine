/*
 * Copyright (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Style/StyleInheritedData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  StyleInheritedData::StyleInheritedData() noexcept
      : BorderSpacing(style::ComputedStyle::InitialBorderSpacing()),
        LineHeight(style::ComputedStyle::InitialLineHeight()), FontData(StyleFontData::Create()),
        Color(style::ComputedStyle::InitialColor())
  {
  }

  inline StyleInheritedData::StyleInheritedData(const StyleInheritedData &o) noexcept
      : RefCounted<StyleInheritedData>(), BorderSpacing(o.BorderSpacing), LineHeight(o.LineHeight),
        FontData(o.FontData), Color(o.Color)
  {
    krys_debug_assert_msg(o == *this, "StyleInheritedData should be properly copied.");
  }

  Ref<StyleInheritedData> StyleInheritedData::Copy() const noexcept
  {
    return AdoptRef(*new StyleInheritedData(*this));
  }

  bool StyleInheritedData::operator==(const StyleInheritedData &other) const noexcept
  {
    return FastPathInheritedEqual(other) && NonFastPathInheritedEqual(other);
  }

  bool StyleInheritedData::FastPathInheritedEqual(const StyleInheritedData &other) const noexcept
  {
    // These properties also need to have "fast-path-inherited" codegen property set.
    // Cases where other properties depend on these values need to disallow the fast path (via
    // RenderStyle::setDisallowsFastPathInheritance).
    return Color == other.Color;
  }

  bool StyleInheritedData::NonFastPathInheritedEqual(const StyleInheritedData &other) const noexcept
  {
    return LineHeight == other.LineHeight && FontData == other.FontData
           && BorderSpacing == other.BorderSpacing;
  }

  void StyleInheritedData::FastPathInheritFrom(const StyleInheritedData &inheritParent) noexcept
  {
    Color = inheritParent.Color;
  }
}
