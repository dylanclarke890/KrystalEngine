/*
 * Copyright (C) 2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2008-2025 Apple Inc. All right reserved.
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

#include "Krystal.Booey/CSS/Values/Primitives/URL.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleCursorImage;
}

namespace krys::boo::style
{
  class BuilderState;
}

namespace krys::boo::css
{
  /// @brief One image of the `cursor` property, with an optional hot spot.
  class CursorImageValue final : public Value
  {
  private:
    CSSURL _originalURL;
    const Ref<Value> _imageValue;
    const RefPtr<Value> _hotSpot;

    CursorImageValue(Ref<Value> &&imageValue, RefPtr<Value> &&hotSpot, CSSURL &&originalURL) noexcept;

  public:
    KRYS_NODISCARD static Ref<CursorImageValue> Create(Ref<Value> &&imageValue,
                                                       RefPtr<Value> &&hotSpot) noexcept;
    KRYS_NODISCARD static Ref<CursorImageValue> Create(Ref<Value> &&imageValue, RefPtr<Value> &&hotSpot,
                                                       CSSURL &&originalURL) noexcept;

    KRYS_NODISCARD const CSSURL &OriginalURL() const noexcept
    {
      return _originalURL;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const CursorImageValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      return func(*_imageValue);
    }

    KRYS_NODISCARD RefPtr<StyleCursorImage> CreateStyleImage(const style::BuilderState &state) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CursorImageValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsCursorImageValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
