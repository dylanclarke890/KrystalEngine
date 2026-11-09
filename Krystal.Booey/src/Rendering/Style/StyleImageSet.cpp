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

#include "Krystal.Booey/Rendering/Style/StyleImageSet.hpp"
#include "Krystal.Booey/CSS/Values/ImageSetOptionValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageSetValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Platform/MIMETypeRegistry.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo
{
  Ref<StyleImageSet> StyleImageSet::Create(List<ImageWithScale> &&images,
                                           List<size_t> &&sortedIndices) noexcept
  {
    krys_debug_assert(images.size() == sortedIndices.size());
    return AdoptRef(*new StyleImageSet(krys::move(images), krys::move(sortedIndices)));
  }

  StyleImageSet::StyleImageSet(List<ImageWithScale> &&images, List<size_t> &&sortedIndices) noexcept
      : StyleMultiImage {Type::ImageSet}, _images {krys::move(images)},
        _sortedIndices {krys::move(sortedIndices)}
  {
  }

  StyleImageSet::~StyleImageSet() noexcept = default;

  bool StyleImageSet::Equals(const StyleImage &other) const noexcept
  {
    const auto *otherImageSet = DynamicDowncast<StyleImageSet>(other);
    return otherImageSet && Equals(*otherImageSet);
  }

  bool StyleImageSet::Equals(const StyleImageSet &other) const noexcept
  {
    return _images == other._images && StyleMultiImage::Equals(other);
  }

  Ref<css::Value> StyleImageSet::ComputedStyleValue(const RenderStyle &style) const noexcept
  {
    css::ValueListBuilder builder;
    for (const auto &image : _images)
    {
      auto resolution = css::PrimitiveValue::Create(image.ScaleFactor, css::UnitType::dppx);
      if (image.MimeType)
      {
        builder.push_back(css::ImageSetOptionValue::Create(image.Image->ComputedStyleValue(style),
                                                           krys::move(resolution), *image.MimeType));
      }
      else
      {
        builder.push_back(
          css::ImageSetOptionValue::Create(image.Image->ComputedStyleValue(style), krys::move(resolution)));
      }
    }
    return css::ImageSetValue::Create(krys::move(builder));
  }

  ImageWithScale StyleImageSet::SelectBestFitImage(const dom::Document &document) noexcept
  {
    UpdateDeviceScaleFactor(document);

    if (!_accessedBestFitImage)
    {
      _accessedBestFitImage = true;
      _bestFitImage = BestImageForScaleFactor();
    }

    return _bestFitImage;
  }

  ImageWithScale StyleImageSet::BestImageForScaleFactor() noexcept
  {
    ImageWithScale result;
    for (auto index : _sortedIndices)
    {
      const auto &image = _images[index];
      if (image.MimeType && !MIMETypeRegistry::IsSupportedImageMIMEType(*image.MimeType))
      {
        continue;
      }
      if (!result.Image->IsInvalidImage() && result.ScaleFactor == image.ScaleFactor)
      {
        continue;
      }
      if (image.ScaleFactor >= _deviceScaleFactor)
      {
        return image;
      }

      result = image;
    }

    krys_debug_assert(result.ScaleFactor >= 0);
    if (result.Image->IsInvalidImage() || !result.ScaleFactor)
    {
      result = ImageWithScale {StyleInvalidImage::Create(), 1, std::nullopt};
    }

    return result;
  }

  void StyleImageSet::UpdateDeviceScaleFactor(const dom::Document &document) noexcept
  {
    // FIXME: In the future, we want to take much more than deviceScaleFactor into acount here.
    // All forms of scale should be included: Page::pageScaleFactor(), Frame::pageZoomFactor(),
    // and any CSS transforms. https://bugs.webkit.org/show_bug.cgi?id=81698
    float deviceScaleFactor = document.DeviceScaleFactor();
    if (deviceScaleFactor == _deviceScaleFactor)
    {
      return;
    }

    _deviceScaleFactor = deviceScaleFactor;
    _accessedBestFitImage = false;
  }
}
