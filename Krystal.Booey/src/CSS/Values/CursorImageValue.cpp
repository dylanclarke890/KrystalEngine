/*
 * Copyright (C) 2006 Rob Buis <buis@kde.org>
 *           (C) 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2008-2021 Apple Inc. All rights reserved.
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Values/CursorImageValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCursorImage.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

namespace krys::boo::css
{
  Ref<CursorImageValue> CursorImageValue::Create(Ref<Value> &&value, RefPtr<Value> &&hotSpot) noexcept
  {
    auto *imageValue = DynamicDowncast<ImageValue>(value.get());
    auto originalURL = imageValue ? imageValue->URL() : CSSURL::None();
    return AdoptRef(*new CursorImageValue(krys::move(value), krys::move(hotSpot), krys::move(originalURL)));
  }

  Ref<CursorImageValue> CursorImageValue::Create(Ref<Value> &&imageValue, RefPtr<Value> &&hotSpot,
                                                 CSSURL &&originalURL) noexcept
  {
    return AdoptRef(
      *new CursorImageValue(krys::move(imageValue), krys::move(hotSpot), krys::move(originalURL)));
  }

  CursorImageValue::CursorImageValue(Ref<Value> &&imageValue, RefPtr<Value> &&hotSpot,
                                     CSSURL &&originalURL) noexcept
      : Value(ValueType::CursorImage), _originalURL(krys::move(originalURL)),
        _imageValue(krys::move(imageValue)), _hotSpot(krys::move(hotSpot))
  {
  }

  CSSOMString CursorImageValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    auto text = _imageValue->CssText(context);
    if (!_hotSpot)
    {
      return text;
    }
    text.push_back(u8' ');
    text.append(_hotSpot->First().CssText(context));
    text.push_back(u8' ');
    text.append(_hotSpot->Second().CssText(context));
    return text;
  }

  bool CursorImageValue::Equals(const CursorImageValue &other) const noexcept
  {
    return CompareValue(_imageValue, other._imageValue) && CompareValuePtr(_hotSpot, other._hotSpot);
  }

  RefPtr<StyleCursorImage> CursorImageValue::CreateStyleImage(const style::BuilderState &state) const noexcept
  {
    auto styleImage = state.CreateStyleImage(*_imageValue);
    if (!styleImage)
    {
      return nullptr;
    }

    Maybe<IntPoint> hotSpot;
    if (_hotSpot)
    {
      // FIXME: Should we clamp or round instead of just casting from double to int?
      hotSpot = IntPoint {
        static_cast<int>(
          Downcast<PrimitiveValue>(_hotSpot->First()).ResolveAsNumber(state.CssToLengthConversionData())),
        static_cast<int>(
          Downcast<PrimitiveValue>(_hotSpot->Second()).ResolveAsNumber(state.CssToLengthConversionData()))};
    }
    return StyleCursorImage::Create(krys::move(styleImage), hotSpot,
                                    style::ConvertToStyle(_originalURL, state));
  }
}
