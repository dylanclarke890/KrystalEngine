/*
 * Copyright (C) 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2009 Google, Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2012 Zoltan Herczeg <zherczeg@webkit.org>.
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/IntRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextSwitcher.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/Painting/CSSFilterRenderer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class GraphicsContext;
  class ImageBuffer;
  class LegacyRenderSVGResourceFilter;
  class RenderElement;
  class RenderObject;
  struct PaintInfo;

  /// @brief The steps `SVGRenderingContext` has taken, so that its destructor can undo them.
  enum class SVGRenderingContextFlag : uint8
  {
    RenderingPrepared,
    RestoreGraphicsContext,
    EndOpacityLayer,
    EndFilterLayer,
    EndCSSFilterLayer,
    PrepareToRenderSVGContentWasCalled
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::SVGRenderingContextFlag, 6uz);

namespace krys::boo
{
  /// @brief Applies an SVG renderer's opacity, blending, clipping, masking and filters around its painting.
  class SVGRenderingContext
  {
    KRYS_NON_COPYABLE(SVGRenderingContext);
    using RenderingFlag = SVGRenderingContextFlag;

  public:
    enum class NeedsGraphicsContextSave : bool
    {
      SaveGraphicsContext,
      DontSaveGraphicsContext
    };

  private:
    RenderElement *_renderer {nullptr};
    PaintInfo *_paintInfo {nullptr};
    GraphicsContext *_savedContext {nullptr};
    LegacyRenderSVGResourceFilter *_filter {nullptr};
    /// @brief The filter functions of a `filter` that isn't one `url()` (`blur()`, `drop-shadow()`, ...), and
    /// the switcher drawing the renderer into their source image.
    RefPtr<CSSFilterRenderer> _cssFilter;
    UniquePtr<GraphicsContextSwitcher> _cssFilterSwitcher;
    LayoutRect _savedPaintRect;
    // To properly revert partially successful initializtions in the destructor, we record all successful
    // steps.
    EnumSet<RenderingFlag> _renderingFlags;
    // True with path-based clipping is known to contrain the clipped area to within the renderer; used to
    // optimize away a context clip.
    bool _pathClippingIsEntirelyWithinRendererContents {false};

  public:
    // Does not start rendering.
    SVGRenderingContext() noexcept = default;

    SVGRenderingContext(SVGRenderingContext &&) noexcept;

    SVGRenderingContext(RenderElement &object, PaintInfo &paintinfo,
                        NeedsGraphicsContextSave needsGraphicsContextSave =
                          NeedsGraphicsContextSave::DontSaveGraphicsContext) noexcept
    {
      PrepareToRenderSVGContent(object, paintinfo, needsGraphicsContextSave);
    }

    // Automatically finishes context rendering.
    ~SVGRenderingContext() noexcept;

    /// @brief Where the filter functions of @p renderer's `filter` (when it isn't one `url()`) draw past its
    /// stroke bounding box, in its local coordinates; none without them.
    KRYS_NODISCARD static IntOutsets CSSFilterOutsets(const RenderElement &renderer) noexcept;

    // Used by all SVG renderers who apply clip/filter/etc. resources to the renderer content.
    void PrepareToRenderSVGContent(
      RenderElement &, PaintInfo &,
      NeedsGraphicsContextSave = NeedsGraphicsContextSave::DontSaveGraphicsContext) noexcept;
    KRYS_NODISCARD bool IsRenderingPrepared() const noexcept
    {
      return _renderingFlags.Contains(RenderingFlag::RenderingPrepared);
    }

    KRYS_NODISCARD bool PathClippingIsEntirelyWithinRendererContents() const noexcept
    {
      return _pathClippingIsEntirelyWithinRendererContents;
    }

    static void RenderSubtreeToContext(GraphicsContext &, RenderElement &, const AffineTransform &) noexcept;

  private:
    void BeginCSSFilter() noexcept;

  public:
    static void ClipToImageBuffer(GraphicsContext &, const FloatRect &targetRect, const FloatSize &scale,
                                  RefPtr<ImageBuffer> &, bool safeToClear) noexcept;

    KRYS_NODISCARD static float CalculateScreenFontSizeScalingFactor(const RenderObject &) noexcept;
    KRYS_NODISCARD static AffineTransform
      CalculateTransformationToOutermostCoordinateSystem(const RenderObject &) noexcept;

    KRYS_NODISCARD static IntRect CalculateImageBufferRect(const FloatRect &targetRect,
                                                           const AffineTransform &absoluteTransform) noexcept
    {
      return EnclosingIntRect(absoluteTransform.MapRect(targetRect));
    }

    KRYS_NODISCARD const RenderElement *Renderer() const noexcept
    {
      return _renderer;
    }
  };
}
