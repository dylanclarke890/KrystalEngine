/*
 * Copyright (C) 1999 Lars Knoll <knoll@kde.org>
 * Copyright (C) 1999 Antti Koivisto <koivisto@kde.org>
 * Copyright (C) 2006 Allan Sandfeld Jensen <kde@carewolf.com>
 * Copyright (C) 2006 Samuel Weinig <sam.weinig@gmail.com>
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Patrick Gansterer <paroga@paroga.com>
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

#include "Krystal.Booey/Rendering/Replaced/RenderImageResource.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The image of generated content (`content: url(...)`): a style image, whatever kind it is.
  class RenderImageResourceStyleImage final : public RenderImageResource
  {
  private:
    Ref<StyleImage> _styleImage;

  public:
    explicit RenderImageResourceStyleImage(StyleImage &styleImage) noexcept;

  private:
    void Initialize(RenderElement &renderer) noexcept override;
    void Shutdown() noexcept override;

    KRYS_NODISCARD RefPtr<Image> GetImage(const IntSize &size = {}) const noexcept override;
    KRYS_NODISCARD bool ErrorOccurred() const noexcept override
    {
      return _styleImage->ErrorOccurred();
    }

    void SetContainerContext(const IntSize &size, const url::URL &) noexcept override;

    KRYS_NODISCARD bool ImageHasRelativeWidth() const noexcept override
    {
      return _styleImage->ImageHasRelativeWidth();
    }
    KRYS_NODISCARD bool ImageHasRelativeHeight() const noexcept override
    {
      return _styleImage->ImageHasRelativeHeight();
    }

    KRYS_NODISCARD WrappedImagePtr ImagePtr() const noexcept override
    {
      return _styleImage->Data();
    }
    KRYS_NODISCARD LayoutSize ImageSize(float multiplier, CachedImage::SizeType) const noexcept override
    {
      return LayoutSize(_styleImage->ImageSize(Renderer(), multiplier));
    }
  };
}
