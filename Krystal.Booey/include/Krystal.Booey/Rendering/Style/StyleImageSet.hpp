/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Rendering/Style/StyleMultiImage.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  /// @brief An `image-set()`: candidates at several resolutions, the best of which is used.
  class StyleImageSet final : public StyleMultiImage
  {
  private:
    bool _accessedBestFitImage {false};
    ImageWithScale _bestFitImage;
    float _deviceScaleFactor {1};
    List<ImageWithScale> _images;
    List<size_t> _sortedIndices;

  public:
    /// @param sortedIndices The indices of `images` from the lowest resolution to the highest.
    KRYS_NODISCARD static Ref<StyleImageSet> Create(List<ImageWithScale> &&images,
                                                    List<size_t> &&sortedIndices) noexcept;
    ~StyleImageSet() noexcept override;

    KRYS_NODISCARD bool Equals(const StyleImage &other) const noexcept final;
    KRYS_NODISCARD bool Equals(const StyleImageSet &) const noexcept;

    KRYS_NODISCARD ImageWithScale SelectBestFitImage(const dom::Document &) noexcept final;

  private:
    StyleImageSet(List<ImageWithScale> &&, List<size_t> &&) noexcept;

    KRYS_NODISCARD Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept final;

    KRYS_NODISCARD ImageWithScale BestImageForScaleFactor() noexcept;
    void UpdateDeviceScaleFactor(const dom::Document &) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleImageSet, IsImageSet)
