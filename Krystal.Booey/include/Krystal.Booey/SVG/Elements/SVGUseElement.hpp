/*
 * Copyright (C) 2004, 2005, 2006, 2007, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2015-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2015 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/Loader/Cache/CachedSVGDocumentClient.hpp"
#include "Krystal.Booey/SVG/Elements/SVGGraphicsElement.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"

namespace krys::boo
{
  class CachedSVGDocument;
}

namespace krys::boo::svg
{
  /// @brief The `<use>` element: draws a copy of another element, cloned into its user agent shadow tree
  /// (its instances) and kept in step with the original.
  /// @see https://svgwg.org/svg2-draft/struct.html#UseElement
  class SVGUseElement final : public SVGGraphicsElement,
                              public SVGURIReference,
                              private CachedSVGDocumentClient
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGUseElement);

  private:
    Ref<SVGAnimatedLength> _x;
    Ref<SVGAnimatedLength> _y;
    Ref<SVGAnimatedLength> _width;
    Ref<SVGAnimatedLength> _height;

    bool _haveFiredLoadEvent {false};
    bool _errorOccurred {false};
    bool _shadowTreeNeedsUpdate {true};
    CachedResourceHandle<CachedSVGDocument> _externalDocument;

  public:
    explicit SVGUseElement(dom::Document &document) noexcept;
    ~SVGUseElement() noexcept override;

    void InvalidateShadowTree() noexcept;
    void UpdateUserAgentShadowTree() noexcept final;

    KRYS_NODISCARD RefPtr<SVGElement> ClipChild() const noexcept;
    KRYS_NODISCARD RenderElement *RendererClipChild() const noexcept;

    KRYS_NODISCARD const SVGLengthValue &X() const noexcept
    {
      return _x->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Y() const noexcept
    {
      return _y->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Width() const noexcept
    {
      return _width->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Height() const noexcept
    {
      return _height->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedLength &XAnimated() noexcept
    {
      return *_x;
    }
    KRYS_NODISCARD SVGAnimatedLength &YAnimated() noexcept
    {
      return *_y;
    }
    KRYS_NODISCARD SVGAnimatedLength &WidthAnimated() noexcept
    {
      return *_width;
    }
    KRYS_NODISCARD SVGAnimatedLength &HeightAnimated() noexcept
    {
      return *_height;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGUseElement, SVGGraphicsElement, SVGURIReference>;

  private:
    void OnInsert() noexcept final;
    void OnPostConnection() noexcept final;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept final;
    void BuildPendingResource() noexcept final;

    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
    KRYS_NODISCARD Path ToClipPath() noexcept final;
    KRYS_NODISCARD bool SelfHasRelativeLengths() const noexcept final;

    void NotifyFinished(CachedResource &) noexcept final;
    KRYS_NODISCARD dom::Document *ExternalDocument() const noexcept;
    void UpdateExternalDocument() noexcept;

    KRYS_NODISCARD FloatRect GetBBox(StyleUpdateStrategy = AllowStyleUpdate) noexcept final;

    KRYS_NODISCARD RefPtr<SVGElement> FindTarget(dom::DOMStringAtom *targetID = nullptr) const noexcept;

    void CloneTarget(dom::ContainerNode &container, SVGElement &target) const noexcept;
    KRYS_NODISCARD RefPtr<SVGElement> TargetClone() const noexcept;

    void ExpandUseElementsInShadowTree() const noexcept;
    void ExpandSymbolElementsInShadowTree() const noexcept;
    void TransferEventListenersToShadowTree() const noexcept;
    void TransferSizeAttributesToTargetClone(SVGElement &shadowElement) const noexcept;

    void ClearShadowTree() noexcept;
    void InvalidateDependentShadowTrees() noexcept;

    KRYS_NODISCARD bool HaveLoadedRequiredResources() noexcept final
    {
      return SVGURIReference::HaveLoadedRequiredResources();
    }
    void SetHaveFiredLoadEvent(bool haveFiredLoadEvent) noexcept final
    {
      _haveFiredLoadEvent = haveFiredLoadEvent;
    }
    KRYS_NODISCARD bool HaveFiredLoadEvent() const noexcept final
    {
      return _haveFiredLoadEvent;
    }
    void SetErrorOccurred(bool errorOccurred) noexcept final
    {
      _errorOccurred = errorOccurred;
    }
    KRYS_NODISCARD bool ErrorOccurred() const noexcept final
    {
      return _errorOccurred;
    }

    KRYS_NODISCARD bool IsValid() const noexcept final
    {
      return SVGTests::IsValid();
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGUseElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGUseElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
