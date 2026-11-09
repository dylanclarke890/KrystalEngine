/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Images/ImagePaintingOptions.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"
#include "Krystal.Booey/SVG/SVGUnitTypes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <array>

namespace krys::boo::svg
{
  class SVGMaskElement;
}

namespace krys::boo
{
  class ImageBuffer;

  /// @brief The mask image a mask drew for one client.
  struct MaskerData
  {
    RefPtr<ImageBuffer> MaskImage;
  };

  /// @brief The renderer of a `<mask>`: draws its children into a mask image its clients are clipped to.
  class LegacyRenderSVGResourceMasker final : public LegacyRenderSVGResourceContainer
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGResourceMasker);

  private:
    std::array<FloatRect, 2> _maskContentBoundaries;
    HashMap<const RenderObject *, UniquePtr<MaskerData>> _masker;

  public:
    LegacyRenderSVGResourceMasker(svg::SVGMaskElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGResourceMasker() noexcept override;

    KRYS_NODISCARD svg::SVGMaskElement &MaskElement() const noexcept;

    void RemoveAllClientsFromCache() noexcept override;
    void RemoveClientFromCache(RenderElement &) noexcept override;
    EnumSet<ApplyResult> ApplyResource(RenderElement &, const RenderStyle &, GraphicsContext *&,
                                       EnumSet<RenderSVGResourceMode>) noexcept override;

    bool DrawContentIntoContext(GraphicsContext &, const FloatRect &objectBoundingBox) noexcept;
    bool DrawContentIntoContext(GraphicsContext &, const FloatRect &destinationRect,
                                const FloatRect &sourceRect, ImagePaintingOptions) noexcept;
    KRYS_NODISCARD FloatRect ResourceBoundingBox(const RenderObject &,
                                                 RepaintRectCalculation) noexcept override;

    KRYS_NODISCARD svg::SVGUnitTypes::SVGUnitType MaskUnits() const noexcept;
    KRYS_NODISCARD svg::SVGUnitTypes::SVGUnitType MaskContentUnits() const noexcept;

    KRYS_NODISCARD RenderSVGResourceType ResourceType() const noexcept override
    {
      return RenderSVGResourceType::MaskerResourceType;
    }

  private:
    void GetElement() const = delete;
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGResourceMasker";
    }

    KRYS_NODISCARD bool DrawContentIntoMaskImage(MaskerData *, RenderObject *) noexcept;
    void CalculateMaskContentRepaintRect(RepaintRectCalculation) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGResourceMasker)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGResourceMasker();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
