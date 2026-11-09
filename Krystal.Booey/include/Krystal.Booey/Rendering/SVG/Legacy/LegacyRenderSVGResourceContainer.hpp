/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#include "Krystal.Booey/DOM/Mixins/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGHiddenContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResource.hpp"
#include "Krystal.Core/Types/HashSet.hpp"

namespace krys::boo
{
  /// @brief A renderer for an element that other renderers paint, clip, mask or filter with: a gradient, a
  /// pattern, a clip path, a mask, a marker or a filter. It never paints itself.
  class LegacyRenderSVGResourceContainer : public LegacyRenderSVGHiddenContainer,
                                           public LegacyRenderSVGResource
  {
    friend class SVGResourcesCache;
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGResourceContainer);

  private:
    dom::DOMStringAtom _id;
    HashSet<RenderElement *> _clients;
    /// @brief The layers filtered from HTML by the resource; each removes itself before it goes.
    HashSet<RenderLayer *> _clientLayers;
    bool _registered {false};
    bool _isInvalidating {false};

  public:
    ~LegacyRenderSVGResourceContainer() noexcept override;

    void Layout() noexcept override;
    void StyleDidChange(style::Difference, const RenderStyle *oldStyle) noexcept final;

    KRYS_NODISCARD static float ComputeTextPaintingScale(const RenderElement &) noexcept;
    KRYS_NODISCARD static AffineTransform
      TransformOnNonScalingStroke(RenderObject *, const AffineTransform &resourceTransform) noexcept;

    void RemoveClientFromCacheAndMarkForInvalidation(RenderElement &,
                                                     bool markForInvalidation = true) noexcept override;
    void RemoveAllClientsFromCacheAndMarkForInvalidationIfNeeded(
      bool markForInvalidation, VisitedRenderers *visitedRenderers) noexcept override;

    void IdChanged() noexcept;
    void MarkAllClientsForRepaint() noexcept;

    void AddClientRenderLayer(RenderLayer &) noexcept;
    void RemoveClientRenderLayer(RenderLayer &) noexcept;
    void MarkAllClientLayersForInvalidation() noexcept;

  protected:
    LegacyRenderSVGResourceContainer(Type, svg::SVGElement &, RenderStyle &&) noexcept;

    enum class InvalidationMode : uint8
    {
      LayoutAndBoundariesInvalidation,
      BoundariesInvalidation,
      RepaintInvalidation,
      ParentOnlyInvalidation
    };

    // Used from the invalidateClient/invalidateClients methods from classes, inheriting from us.
    KRYS_NODISCARD virtual bool SelfNeedsClientInvalidation() const noexcept
    {
      return EverHadLayout() && SelfNeedsLayout();
    }

    void MarkAllClientsForInvalidation(InvalidationMode) noexcept;
    void MarkAllClientsForInvalidationIfNeeded(InvalidationMode, VisitedRenderers *visitedRenderers) noexcept;
    void MarkClientForInvalidation(RenderObject &, InvalidationMode) noexcept;

  private:
    void AddClient(RenderElement &) noexcept;
    void RemoveClient(RenderElement &) noexcept;

    void WillBeDestroyed() noexcept final;
    void RegisterResource() noexcept;
  };

  KRYS_NODISCARD inline LegacyRenderSVGResourceContainer *
    GetRenderSVGResourceContainerById(dom::DocumentOrShadowRoot &treeScope,
                                      const dom::DOMStringAtom &id) noexcept
  {
    if (id.View().empty())
    {
      return nullptr;
    }

    if (auto *renderResource = treeScope.LookupLegacySVGResourceById(id))
    {
      return renderResource;
    }

    return nullptr;
  }

  template <typename TRenderer>
  KRYS_NODISCARD TRenderer *GetRenderSVGResourceById(dom::DocumentOrShadowRoot &treeScope,
                                                     const dom::DOMStringAtom &id) noexcept
  {
    return DynamicDowncast<TRenderer>(GetRenderSVGResourceContainerById(treeScope, id));
  }
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGResourceContainer)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGResourceContainer();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
