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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"
#include "Krystal.Web/Html/Types/TagName.hpp"
#include <utility>

namespace krys::web::style
{
  class ReferenceFilterOperation;
  struct ReferencePath;
}

namespace krys::web
{
  class CSSSVGResourceElementClient;
  class Document;
  class DocumentOrShadowRoot;
  class LegacyRenderSVGResourceClipper;
  class LegacyRenderSVGResourceContainer;
  class RenderElement;
  class RenderStyle;
  class SVGElement;
  class SVGFilterElement;

  /// @brief The SVG elements a renderer's style refers to (a `clip-path: url()` or `filter: url()`), which
  /// tell it when they change.
  class ReferencedSVGResources
  {
    KRYS_NON_COPY_MOVABLE(ReferencedSVGResources);

  public:
    using SVGQualifiedNames = List<html::TagName>;
    using SVGElementIdentifierAndTagPairs = List<std::pair<DOMStringAtom, SVGQualifiedNames>>;

  private:
    RenderElement &_renderer;
    HashMap<DOMStringAtom, UniquePtr<CSSSVGResourceElementClient>> _elementClients;

  public:
    explicit ReferencedSVGResources(RenderElement &) noexcept;

    ~ReferencedSVGResources() noexcept;

    KRYS_NODISCARD static SVGElementIdentifierAndTagPairs ReferencedSVGResourceIDs(const RenderStyle &,
                                                                                   const Document &) noexcept;

    void UpdateReferencedResources(DocumentOrShadowRoot &, const SVGElementIdentifierAndTagPairs &) noexcept;

    // Legacy: Clipping needs a renderer, filters use an element.
    KRYS_NODISCARD static LegacyRenderSVGResourceClipper *
      ReferencedClipperRenderer(DocumentOrShadowRoot &, const style::ReferencePath &) noexcept;

    KRYS_NODISCARD static RefPtr<SVGFilterElement>
      ReferencedFilterElement(DocumentOrShadowRoot &, const style::ReferenceFilterOperation &) noexcept;

    KRYS_NODISCARD static LegacyRenderSVGResourceContainer *
      ReferencedRenderResource(DocumentOrShadowRoot &, const DOMStringAtom &fragment) noexcept;

  private:
    KRYS_NODISCARD static RefPtr<SVGElement>
      ElementForResourceID(DocumentOrShadowRoot &, const DOMStringAtom &resourceID, html::TagName) noexcept;

    KRYS_NODISCARD static RefPtr<SVGElement> ElementForResourceIDs(DocumentOrShadowRoot &,
                                                                   const DOMStringAtom &resourceID,
                                                                   const SVGQualifiedNames &) noexcept;

    void AddClientForTarget(SVGElement &targetElement, const DOMStringAtom &) noexcept;

    void RemoveClientForTarget(DocumentOrShadowRoot &, const DOMStringAtom &) noexcept;
  };
}
