/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2018-2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Loader/Cache/CachedImageClient.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"

namespace krys::boo
{
  class CachedImage;
  class ImageBuffer;

  /// @brief The `<feImage>` element: an external image, or an element of the document, drawn into its
  /// subregion.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEImageElement
  class SVGFEImageElement final : public SVGFilterPrimitiveStandardAttributes,
                                  public SVGURIReference,
                                  private CachedImageClient
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEImageElement);

  private:
    Ref<SVGAnimatedPreserveAspectRatio> _preserveAspectRatio;
    CachedResourceHandle<CachedImage> _cachedImage;

  public:
    explicit SVGFEImageElement(Document &document) noexcept;

    ~SVGFEImageElement() noexcept override;

    KRYS_NODISCARD bool RenderingTaintsOrigin() const noexcept;

    KRYS_NODISCARD const SVGPreserveAspectRatioValue &PreserveAspectRatio() const noexcept
    {
      return _preserveAspectRatio->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedPreserveAspectRatio &PreserveAspectRatioAnimated() noexcept
    {
      return *_preserveAspectRatio;
    }

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGFEImageElement, SVGFilterPrimitiveStandardAttributes, SVGURIReference>;

  private:
    void AttributeChanged(const QualifiedName &name, Maybe<DOMStringView> oldValue,
                          Maybe<DOMStringView> value) noexcept final;

    void SVGAttributeChanged(const QualifiedName &attributeName) noexcept final;

    void NotifyFinished(CachedResource &) noexcept final;

    void OnPostConnection() noexcept final;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept final;

    struct ImageBufferForEffect
    {
      RefPtr<ImageBuffer> Buffer;
      RectF ImageRect;
    };

    KRYS_NODISCARD ImageBufferForEffect
      GetImageBufferForEffect(const GraphicsContext &destinationContext) const noexcept;

    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateFilterEffect(const FilterEffectVector &inputs,
                         const GraphicsContext &destinationContext) const noexcept final;

    void ClearResourceReferences() noexcept;

    void RequestImageResource() noexcept;

    void BuildPendingResource() noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SVGFEImageElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::SVGElement &target) noexcept
  {
    return target.IsSVGFEImageElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
