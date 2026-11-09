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
  /// @brief Four side values that serialise in the shortest form, e.g. `1px 2px` for `1px 2px 1px 2px`.
  class Quad final : public RectBase
  {
  public:
    explicit Quad(Ref<Value> value) noexcept : RectBase(krys::move(value))
    {
    }

    Quad(Ref<Value> top, Ref<Value> right, Ref<Value> bottom, Ref<Value> left) noexcept
        : RectBase(krys::move(top), krys::move(right), krys::move(bottom), krys::move(left))
    {
    }

    KRYS_NODISCARD CSSOMString CssText(const SerialisationContext &context) const noexcept
    {
      return Serialise(Top().CssText(context), Right().CssText(context), Bottom().CssText(context),
                       Left().CssText(context));
    }

    /// @brief Serialises four side values, omitting the trailing ones that repeat their opposite side.
    KRYS_NODISCARD static CSSOMString Serialise(const CSSOMString &top, const CSSOMString &right,
                                                const CSSOMString &bottom, const CSSOMString &left) noexcept
    {
      if (left != right)
      {
        return top + u8' ' + right + u8' ' + bottom + u8' ' + left;
      }

      if (bottom != top)
      {
        return top + u8' ' + right + u8' ' + bottom;
      }

      if (right != top)
      {
        return top + u8' ' + right;
      }

      return top;
    }
  };
}
