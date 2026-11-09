/*
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2015-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2009-2015 Google, Inc.
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

#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGContainer.hpp"

namespace krys::boo::svg
{
  class SVGGraphicsElement;
}

namespace krys::boo
{
  /// @brief The renderer of a transformable SVG container: `<g>`, `<a>`, `<switch>` and `<use>`.
  class LegacyRenderSVGTransformableContainer final : public LegacyRenderSVGContainer
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGTransformableContainer);

  private:
    bool _needsTransformUpdate : 1 {true};
    bool _didTransformToRootUpdate : 1 {false};
    AffineTransform _localTransform {};
    FloatSize _additionalTranslation {};
    FloatRect _lastTransformReferenceBoxRect {};

  public:
    LegacyRenderSVGTransformableContainer(svg::SVGGraphicsElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGTransformableContainer() noexcept override;

    KRYS_NODISCARD const AffineTransform &LocalToParentTransform() const noexcept override
    {
      return _localTransform;
    }
    void SetNeedsTransformUpdate() noexcept override
    {
      _needsTransformUpdate = true;
    }
    KRYS_NODISCARD bool DidTransformToRootUpdate() noexcept override
    {
      return _didTransformToRootUpdate;
    }
    KRYS_NODISCARD const FloatSize &AdditionalTranslation() const noexcept
    {
      return _additionalTranslation;
    }

  private:
    KRYS_NODISCARD svg::SVGGraphicsElement &GraphicsElement() noexcept;
    void GetElement() const = delete;

    bool CalculateLocalTransform() noexcept override;
    KRYS_NODISCARD AffineTransform LocalTransform() const noexcept override
    {
      return _localTransform;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGTransformableContainer)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGTransformableContainer();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
