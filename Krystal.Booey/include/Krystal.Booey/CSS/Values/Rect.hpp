/**
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004-2023 Apple Inc. All rights reserved.
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
 */

#pragma once

#include "Krystal.Booey/CSS/Values/RectBase.hpp"

namespace krys::boo::css
{
  /// @brief The four sides of a `rect()`, e.g. `clip: rect(1px, 2px, 3px, 4px)`.
  class Rect final : public RectBase
  {
  public:
    Rect(Ref<Value> top, Ref<Value> right, Ref<Value> bottom, Ref<Value> left) noexcept
        : RectBase(krys::move(top), krys::move(right), krys::move(bottom), krys::move(left))
    {
    }

    KRYS_NODISCARD CSSOMString CssText(const SerialisationContext &context) const noexcept
    {
      return GenerateCSSString(Top().CssText(context), Right().CssText(context), Bottom().CssText(context),
                               Left().CssText(context));
    }

  private:
    KRYS_NODISCARD static CSSOMString GenerateCSSString(const CSSOMString &top, const CSSOMString &right,
                                                        const CSSOMString &bottom,
                                                        const CSSOMString &left) noexcept
    {
      return u8"rect(" + top + u8", " + right + u8", " + bottom + u8", " + left + u8')';
    }
  };
}
