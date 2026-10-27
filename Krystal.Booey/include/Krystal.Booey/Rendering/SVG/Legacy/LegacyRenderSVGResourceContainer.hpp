#pragma once

#include "Krystal.Booey/DOM/DocumentOrShadowRoot.hpp"
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
    // FIXME: The layers clipped, masked or filtered from HTML (m_clientLayers, addClientRenderLayer,
    // removeClientRenderLayer, markAllClientLayersForInvalidation), with clipping and masking from HTML
    // (B11f).
    bool _registered {false};
    bool _isInvalidating {false};

  public:
    ~LegacyRenderSVGResourceContainer() noexcept override;

    void Layout() noexcept override;
    void StyleDidChange(style::Difference, const RenderStyle *oldStyle) noexcept final;

    // FIXME: computeTextPaintingScale, with text (B11g).
    KRYS_NODISCARD static AffineTransform
      TransformOnNonScalingStroke(RenderObject *, const AffineTransform &resourceTransform) noexcept;

    void RemoveClientFromCacheAndMarkForInvalidation(RenderElement &,
                                                     bool markForInvalidation = true) noexcept override;
    void RemoveAllClientsFromCacheAndMarkForInvalidationIfNeeded(
      bool markForInvalidation, VisitedRenderers *visitedRenderers) noexcept override;

    void IdChanged() noexcept;
    void MarkAllClientsForRepaint() noexcept;

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

  // FIXME: getRenderSVGResourceById, typed by the resource type, with clipping and masking (B11f).
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGResourceContainer)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGResourceContainer();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
