#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/HTML/Enums/TagName.hpp"
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
