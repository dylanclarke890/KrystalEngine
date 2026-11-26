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

#pragma once

#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Rendering/Style/StyleMultiImage.hpp"

namespace krys::web
{
  /// @brief An image in a `cursor` value, with its hot spot.
  class StyleCursorImage final : public StyleMultiImage
  {
  private:
    Ref<StyleImage> _image;
    Maybe<PointI> _hotSpot;
    style::Url _originalUrl;

  public:
    KRYS_NODISCARD static Ref<StyleCursorImage> Create(const Ref<StyleImage> &, Maybe<PointI>,
                                                       const style::Url &) noexcept;

    KRYS_NODISCARD static Ref<StyleCursorImage> Create(Ref<StyleImage> &&, Maybe<PointI>,
                                                       style::Url &&) noexcept;

    ~StyleCursorImage() noexcept override;

    KRYS_NODISCARD bool Equals(const StyleImage &) const noexcept final;

    KRYS_NODISCARD bool Equals(const StyleCursorImage &) const noexcept;

    KRYS_NODISCARD bool EqualInputImages(const StyleCursorImage &) const noexcept;

    KRYS_NODISCARD bool UsesDataProtocol() const noexcept final;

    KRYS_NODISCARD Maybe<PointI> HotSpot() const noexcept
    {
      return _hotSpot;
    }

  private:
    StyleCursorImage(const Ref<StyleImage> &, Maybe<PointI> hotSpot, const style::Url &) noexcept;

    StyleCursorImage(Ref<StyleImage> &&, Maybe<PointI> hotSpot, style::Url &&) noexcept;

    KRYS_NODISCARD Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept final;

    KRYS_NODISCARD ImageWithScale SelectBestFitImage(const Document &) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleCursorImage, IsCursorImage)
