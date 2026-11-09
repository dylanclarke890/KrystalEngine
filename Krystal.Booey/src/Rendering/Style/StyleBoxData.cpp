/*
 * Copyright (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
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
