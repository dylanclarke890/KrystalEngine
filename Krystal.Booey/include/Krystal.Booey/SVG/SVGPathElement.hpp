#pragma once

#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathByteStream.hpp"
#include "Krystal.Booey/SVG/SVGGeometryElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<path>` element: a shape drawn by path data, from its `d` attribute or the `d` property.
  /// @see https://svgwg.org/svg2-draft/paths.html#InterfaceSVGPathElement
  class SVGPathElement final : public SVGGeometryElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGPathElement);

  private:
    // The path data of the `d` attribute.
    SVGPathByteStream _pathByteStream;

  public:
    explicit SVGPathElement(dom::Document &document) noexcept;

    KRYS_NODISCARD float GetTotalLength() const noexcept final;
    KRYS_NODISCARD dom::ExceptionOr<Ref<SVGPoint>> GetPointAtLength(float distance) const noexcept final;

    KRYS_NODISCARD FloatRect GetBBox(StyleUpdateStrategy = AllowStyleUpdate) noexcept final;

    /// @brief The path data the element is drawn with: the `d` property's once it has a renderer, otherwise
    /// the `d` attribute's.
    KRYS_NODISCARD const SVGPathByteStream &PathByteStream() const noexcept;
    KRYS_NODISCARD Path GetPath() const noexcept;

    void PathDidChange() noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGPathElement, SVGGeometryElement>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    KRYS_NODISCARD bool IsValid() const noexcept final
    {
      return SVGTests::IsValid();
    }
    KRYS_NODISCARD bool SupportsMarkers() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;

    // FIXME: Tell <mpath> elements referencing the path when it changes, is inserted or is removed
    // (invalidateMPathDependencies), with SMIL (B11i).

    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept final;
    void CollectExtraStyleForPresentationalHints(css::MutableStyleProperties &style) noexcept override;
    void CollectDPresentationalHint(css::MutableStyleProperties &style) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGPathElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGPathElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
