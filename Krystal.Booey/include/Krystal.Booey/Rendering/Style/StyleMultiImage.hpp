/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003-2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Rendering/Style/StyleInvalidImage.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  /// @brief One of the candidate images of an image set or cursor, with its resolution.
  struct ImageWithScale
  {
    RefPtr<StyleImage> Image {StyleInvalidImage::Create()};
    float ScaleFactor {1};
    Maybe<css::CSSOMString> MimeType {};

    KRYS_NODISCARD bool operator==(const ImageWithScale &other) const noexcept
    {
      return Image == other.Image && ScaleFactor == other.ScaleFactor;
    }
  };

  /// @brief An image that selects one of several candidates when it loads.
  class StyleMultiImage : public StyleImage
  {
  private:
    RefPtr<StyleImage> _selectedImage;
    bool _isPending {true};

  public:
    ~StyleMultiImage() noexcept override;

  protected:
    explicit StyleMultiImage(Type) noexcept;

    KRYS_NODISCARD bool Equals(const StyleMultiImage &other) const noexcept;

    KRYS_NODISCARD virtual ImageWithScale SelectBestFitImage(const dom::Document &) noexcept = 0;
    KRYS_NODISCARD CachedImage *GetCachedImage() const noexcept final;

  private:
    KRYS_NODISCARD WrappedImagePtr Data() const noexcept final;
    void Load(CachedResourceLoader &, const ResourceLoaderOptions &) noexcept final;
    KRYS_NODISCARD bool CanRender(const RenderElement *, float multiplier) const noexcept final;
    KRYS_NODISCARD bool IsPending() const noexcept final;
    KRYS_NODISCARD bool IsLoaded(const RenderElement *) const noexcept final;
    KRYS_NODISCARD bool ErrorOccurred() const noexcept final;
    KRYS_NODISCARD FloatSize ImageSize(const RenderElement *, float multiplier) const noexcept final;
    KRYS_NODISCARD bool ImageHasRelativeWidth() const noexcept final;
    KRYS_NODISCARD bool ImageHasRelativeHeight() const noexcept final;
    void ComputeIntrinsicDimensions(const RenderElement *, float &intrinsicWidth, float &intrinsicHeight,
                                    FloatSize &intrinsicRatio) noexcept final;
    KRYS_NODISCARD bool UsesImageContainerSize() const noexcept final;
    void SetContainerContextForRenderer(const RenderElement &, const FloatSize &, float) noexcept final;
    void AddClient(RenderElement &) noexcept final;
    void RemoveClient(RenderElement &) noexcept final;
    KRYS_NODISCARD bool HasClient(RenderElement &) const noexcept final;
    KRYS_NODISCARD RefPtr<Image> GetImage(const RenderElement *, const FloatSize &, const GraphicsContext &,
                                          bool isForFirstLine) const noexcept final;
    KRYS_NODISCARD float ImageScaleFactor() const noexcept final;
    KRYS_NODISCARD bool KnownToBeOpaque(const RenderElement &) const noexcept final;

    KRYS_NODISCARD const StyleImage *SelectedImage() const noexcept final
    {
      return _selectedImage.get();
    }
    KRYS_NODISCARD StyleImage *SelectedImage() noexcept final
    {
      return _selectedImage.get();
    }
  };
}
