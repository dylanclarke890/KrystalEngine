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

#include "Krystal.Booey/Rendering/Style/StyleSurroundData.hpp"

#include "Krystal.Core/Threading/NeverDestroyed.hpp"

namespace krys::boo
{
  using namespace css::Literals;

  StyleSurroundData::StyleSurroundData() noexcept
      : HasExplicitlySetBorderBottomLeftRadius(false), HasExplicitlySetBorderBottomRightRadius(false),
        HasExplicitlySetBorderTopLeftRadius(false), HasExplicitlySetBorderTopRightRadius(false),
        HasExplicitlySetPaddingBottom(false), HasExplicitlySetPaddingLeft(false),
        HasExplicitlySetPaddingRight(false), HasExplicitlySetPaddingTop(false), Inset(css::keywords::Auto {}),
        Margin(0_css_px), Padding(0_css_px)
  {
  }

  inline StyleSurroundData::StyleSurroundData(const StyleSurroundData &o) noexcept
      : RefCounted<StyleSurroundData>(),
        HasExplicitlySetBorderBottomLeftRadius(o.HasExplicitlySetBorderBottomLeftRadius),
        HasExplicitlySetBorderBottomRightRadius(o.HasExplicitlySetBorderBottomRightRadius),
        HasExplicitlySetBorderTopLeftRadius(o.HasExplicitlySetBorderTopLeftRadius),
        HasExplicitlySetBorderTopRightRadius(o.HasExplicitlySetBorderTopRightRadius),
        HasExplicitlySetPaddingBottom(o.HasExplicitlySetPaddingBottom),
        HasExplicitlySetPaddingLeft(o.HasExplicitlySetPaddingLeft),
        HasExplicitlySetPaddingRight(o.HasExplicitlySetPaddingRight),
        HasExplicitlySetPaddingTop(o.HasExplicitlySetPaddingTop), Inset(o.Inset), Margin(o.Margin),
        Padding(o.Padding), Border(o.Border)
  {
  }

  Ref<StyleSurroundData> StyleSurroundData::Copy() const noexcept
  {
    return AdoptRef(*new StyleSurroundData(*this));
  }

  bool StyleSurroundData::operator==(const StyleSurroundData &o) const noexcept
  {
    return Inset == o.Inset && Margin == o.Margin && Padding == o.Padding && Border == o.Border
           && HasExplicitlySetBorderBottomLeftRadius == o.HasExplicitlySetBorderBottomLeftRadius
           && HasExplicitlySetBorderBottomRightRadius == o.HasExplicitlySetBorderBottomRightRadius
           && HasExplicitlySetBorderTopLeftRadius == o.HasExplicitlySetBorderTopLeftRadius
           && HasExplicitlySetBorderTopRightRadius == o.HasExplicitlySetBorderTopRightRadius
           && HasExplicitlySetPaddingBottom == o.HasExplicitlySetPaddingBottom
           && HasExplicitlySetPaddingLeft == o.HasExplicitlySetPaddingLeft
           && HasExplicitlySetPaddingRight == o.HasExplicitlySetPaddingRight
           && HasExplicitlySetPaddingTop == o.HasExplicitlySetPaddingTop;
  }
}
