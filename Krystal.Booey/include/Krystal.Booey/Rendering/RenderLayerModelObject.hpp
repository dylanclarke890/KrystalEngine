/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2006, 2007, 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2010, 2012 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class LegacyRenderSVGResourceClipper;

  /// @brief A renderer that may have a layer.
  class RenderLayerModelObject : public RenderElement
  {
    KRYS_NON_COPY_MOVABLE(RenderLayerModelObject);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderLayerModelObject);

  private:
    UniquePtr<RenderLayer> _layer;

  public:
    ~RenderLayerModelObject() noexcept override;

    void DestroyLayer() noexcept;

    KRYS_NODISCARD RenderLayer *Layer() const noexcept
    {
      return _layer.get();
    }

    KRYS_NODISCARD Maybe<LayoutRect> CachedLayerClippedOverflowRect() const noexcept;

    void StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept override;
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

    KRYS_NODISCARD virtual bool RequiresLayer() const noexcept = 0;

    bool StartAnimation(double timeOffset, const GraphicsLayerAnimation &animation,
                        const BlendingKeyframes &keyframes) noexcept override;
    void AnimationPaused(double timeOffset, const BlendingKeyframes &keyframes) noexcept override;
    void AnimationFinished(const BlendingKeyframes &keyframes) noexcept override;
    void TransformRelatedPropertyDidChange() noexcept override;
    void SuspendAnimations(MonotonicTime::duration time = {}) noexcept override;

    KRYS_NODISCARD TransformationMatrix *LayerTransform() const noexcept;

    virtual void UpdateLayerTransform() noexcept;
    virtual void ApplyTransform(TransformationMatrix &transform, const RenderStyle &style,
                                const FloatRect &boundingBox,
                                EnumSet<style::TransformResolverOption> options) const noexcept = 0;
    void ApplyTransform(TransformationMatrix &transform, const RenderStyle &style,
                        const FloatRect &boundingBox) const noexcept;

    /// @brief Clips @p rects (in this renderer's coordinates, its contents scrolled) to its overflow clip, as
    /// @p context says, for mapping them up to @p container; returns whether anything's left.
    virtual bool ApplyCachedClipAndScrollPosition(RepaintRects &rects,
                                                  const RenderLayerModelObject *container,
                                                  VisibleRectContext context) const noexcept;

    KRYS_NODISCARD LegacyRenderSVGResourceClipper *LegacySVGClipperResourceFromStyle() const noexcept;

    KRYS_NODISCARD bool ShouldUsePositionedClipping() const noexcept
    {
      return IsAbsolutelyPositioned();
    }

  protected:
    RenderLayerModelObject(Type type, dom::Element &element, RenderStyle &&style,
                           EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept;
    RenderLayerModelObject(Type type, dom::Document &document, RenderStyle &&style,
                           EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept;

    void CreateLayer() noexcept;
    void WillBeDestroyed() noexcept override;

    /// @brief Sets the state bits that follow from the renderer's style.
    virtual void UpdateFromStyle() noexcept
    {
    }
  };

  /// @brief Snaps @p rect to device pixels, as every renderer is (there are no layer-aware SVG renderers).
  KRYS_NODISCARD FloatRect SnapRectToDevicePixelsIfNeeded(const LayoutRect &rect,
                                                          const RenderLayerModelObject &renderer) noexcept;
  KRYS_NODISCARD FloatRect SnapRectToDevicePixelsIfNeeded(const FloatRect &rect,
                                                          const RenderLayerModelObject &renderer) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderLayerModelObject)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderLayerModelObject();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
