/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2005-2008, 2016 Apple Inc. All rights reserved.
 * Copyright (C) 2020 Noam Rosenthal (noam@webkit.org)
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

#include "Krystal.Booey/Rendering/Style/StyleCursorImage.hpp"
#include "Krystal.Booey/CSS/Values/CursorImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImageSet.hpp"
#include "Krystal.Core/Utils/PointerComparison.hpp"

namespace krys::boo
{
  Ref<StyleCursorImage> StyleCursorImage::Create(const Ref<StyleImage> &image, Maybe<IntPoint> hotSpot,
                                                 const style::URL &originalURL) noexcept
  {
    return AdoptRef(*new StyleCursorImage(image, hotSpot, originalURL));
  }

  Ref<StyleCursorImage> StyleCursorImage::Create(Ref<StyleImage> &&image, Maybe<IntPoint> hotSpot,
                                                 style::URL &&originalURL) noexcept
  {
    return AdoptRef(*new StyleCursorImage(krys::move(image), hotSpot, krys::move(originalURL)));
  }

  StyleCursorImage::StyleCursorImage(const Ref<StyleImage> &image, Maybe<IntPoint> hotSpot,
                                     const style::URL &originalURL) noexcept
      : StyleMultiImage {Type::CursorImage}, _image {image}, _hotSpot {hotSpot}, _originalURL {originalURL}
  {
  }

  StyleCursorImage::StyleCursorImage(Ref<StyleImage> &&image, Maybe<IntPoint> hotSpot,
                                     style::URL &&originalURL) noexcept
      : StyleMultiImage {Type::CursorImage}, _image {krys::move(image)}, _hotSpot {hotSpot},
        _originalURL {krys::move(originalURL)}
  {
  }

  StyleCursorImage::~StyleCursorImage() noexcept = default;

  bool StyleCursorImage::Equals(const StyleImage &other) const noexcept
  {
    const auto *otherCursorImage = DynamicDowncast<StyleCursorImage>(other);
    return otherCursorImage && Equals(*otherCursorImage);
  }

  bool StyleCursorImage::Equals(const StyleCursorImage &other) const noexcept
  {
    return EqualInputImages(other) && StyleMultiImage::Equals(other);
  }

  bool StyleCursorImage::EqualInputImages(const StyleCursorImage &other) const noexcept
  {
    return ArePointingToEqualData(_image, other._image);
  }

  Ref<css::Value> StyleCursorImage::ComputedStyleValue(const RenderStyle &style) const noexcept
  {
    RefPtr<css::Value> hotSpot;
    if (_hotSpot)
    {
      hotSpot = css::ValuePair::CreateNonCoalescing(css::PrimitiveValue::Create(_hotSpot->X()),
                                                    css::PrimitiveValue::Create(_hotSpot->Y()));
    }
    return css::CursorImageValue::Create(_image->ComputedStyleValue(style), krys::move(hotSpot),
                                         style::ConvertToCSS(_originalURL, style));
  }

  ImageWithScale StyleCursorImage::SelectBestFitImage(const dom::Document &document) noexcept
  {
    if (auto *imageSet = DynamicDowncast<StyleImageSet>(_image.get()))
    {
      return imageSet->SelectBestFitImage(document);
    }

    return {RefPtr<StyleImage> {_image}, 1, std::nullopt};
  }

  bool StyleCursorImage::UsesDataProtocol() const noexcept
  {
    return _originalURL.Resolved.ProtocolIsData();
  }
}
