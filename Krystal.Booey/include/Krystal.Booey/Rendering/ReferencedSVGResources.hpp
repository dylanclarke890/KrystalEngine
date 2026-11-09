/*
 * Copyright (C) 2021-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/HTML/Types/TagName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <utility>

namespace krys::boo::dom
{
  class Document;
  class DocumentOrShadowRoot;
}

namespace krys::boo::style
{
  class ReferenceFilterOperation;
  struct ReferencePath;
}

namespace krys::boo::svg
{
  class SVGElement;
  class SVGFilterElement;
}

namespace krys::boo
{
  class CSSSVGResourceElementClient;
  class LegacyRenderSVGResourceClipper;
  class LegacyRenderSVGResourceContainer;
  class RenderElement;
  class RenderStyle;

  /// @brief The SVG elements a renderer's style refers to (a `clip-path: url()` or `filter: url()`), which
  /// tell it when they change.
  class ReferencedSVGResources
  {
    KRYS_NON_COPY_MOVABLE(ReferencedSVGResources);

  public:
    using SVGQualifiedNames = List<html::TagName>;
    using SVGElementIdentifierAndTagPairs = List<std::pair<dom::DOMStringAtom, SVGQualifiedNames>>;

  private:
    RenderElement &_renderer;
    HashMap<dom::DOMStringAtom, UniquePtr<CSSSVGResourceElementClient>> _elementClients;

  public:
    explicit ReferencedSVGResources(RenderElement &) noexcept;
    ~ReferencedSVGResources() noexcept;

    KRYS_NODISCARD static SVGElementIdentifierAndTagPairs
      ReferencedSVGResourceIDs(const RenderStyle &, const dom::Document &) noexcept;
    void UpdateReferencedResources(dom::DocumentOrShadowRoot &,
                                   const SVGElementIdentifierAndTagPairs &) noexcept;

    // Legacy: Clipping needs a renderer, filters use an element.
    KRYS_NODISCARD static LegacyRenderSVGResourceClipper *
      ReferencedClipperRenderer(dom::DocumentOrShadowRoot &, const style::ReferencePath &) noexcept;
    KRYS_NODISCARD static RefPtr<svg::SVGFilterElement>
      ReferencedFilterElement(dom::DocumentOrShadowRoot &, const style::ReferenceFilterOperation &) noexcept;

    KRYS_NODISCARD static LegacyRenderSVGResourceContainer *
      ReferencedRenderResource(dom::DocumentOrShadowRoot &, const dom::DOMStringAtom &fragment) noexcept;

  private:
    KRYS_NODISCARD static RefPtr<svg::SVGElement> ElementForResourceID(dom::DocumentOrShadowRoot &,
                                                                       const dom::DOMStringAtom &resourceID,
                                                                       html::TagName) noexcept;
    KRYS_NODISCARD static RefPtr<svg::SVGElement> ElementForResourceIDs(dom::DocumentOrShadowRoot &,
                                                                        const dom::DOMStringAtom &resourceID,
                                                                        const SVGQualifiedNames &) noexcept;

    void AddClientForTarget(svg::SVGElement &targetElement, const dom::DOMStringAtom &) noexcept;
    void RemoveClientForTarget(dom::DocumentOrShadowRoot &, const dom::DOMStringAtom &) noexcept;
  };
}
