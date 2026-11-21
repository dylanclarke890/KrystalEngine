/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003-2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"

namespace krys::boo
{
  class GeneratedImage;

  /// @brief An image generated from its CSS description rather than loaded. Those that can be (gradients)
  /// keep the images they make, per size, for a few seconds after they were last painted.
  class StyleGeneratedImage : public StyleImage
  {
  private:
    class CachedGeneratedImage;

  protected:
    SizeF _containerSize;

  private:
    bool _fixedSize;
    // Counted: a renderer can use an image more than once.
    HashMap<RenderElement *, uint32> _clients;
    // Few sizes are painted at once, so a list is enough.
    List<UniquePtr<CachedGeneratedImage>> _images;

  protected:
    StyleGeneratedImage(StyleImage::Type, bool fixedSize) noexcept;

    ~StyleGeneratedImage() noexcept override;

    KRYS_NODISCARD WrappedImagePtr Data() const noexcept final
    {
      return this;
    }

    KRYS_NODISCARD bool ImageHasRelativeWidth() const noexcept final
    {
      return !_fixedSize;
    }

    KRYS_NODISCARD bool ImageHasRelativeHeight() const noexcept final
    {
      return !_fixedSize;
    }

    KRYS_NODISCARD bool UsesImageContainerSize() const noexcept final
    {
      return !_fixedSize;
    }

    KRYS_NODISCARD bool ImageHasNaturalDimensions() const noexcept final
    {
      return !UsesImageContainerSize();
    }

    KRYS_NODISCARD SizeF ImageSize(const RenderElement *, float multiplier) const noexcept final;

    void AddClient(RenderElement &) noexcept final;

    void RemoveClient(RenderElement &) noexcept final;

    KRYS_NODISCARD bool HasClient(RenderElement &) const noexcept final;

    KRYS_NODISCARD const HashMap<RenderElement *, uint32> &Clients() const noexcept
    {
      return _clients;
    }

    virtual void DidAddClient(RenderElement &) noexcept
    {
    }

    virtual void DidRemoveClient(RenderElement &) noexcept
    {
    }

    void ComputeIntrinsicDimensions(const RenderElement *, float &intrinsicWidth, float &intrinsicHeight,
                                    SizeF &intrinsicRatio) noexcept final;

    void SetContainerContextForRenderer(const RenderElement &, const SizeF &containerSize,
                                        float) noexcept final
    {
      _containerSize = containerSize;
    }

    /// @brief The image made for @p size, if it's still kept; it's then kept for longer.
    KRYS_NODISCARD GeneratedImage *CachedImageForSize(SizeF size) noexcept;

    /// @brief Keeps @p image, made for @p size, until it hasn't been asked for in a few seconds.
    void SaveCachedImageForSize(SizeF size, GeneratedImage &image) noexcept;

    /// @brief The image's size when it's fixed-size (it doesn't take the size of its container).
    KRYS_NODISCARD virtual SizeF FixedSize(const RenderElement &) const noexcept = 0;

  private:
    void EvictCachedGeneratedImage(const CachedGeneratedImage &) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleGeneratedImage, IsGeneratedImage)
