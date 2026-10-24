#pragma once

#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/HTML/EventHandlerAttributes.hpp"
#include "Krystal.Booey/HTML/Mixins/HTMLOrSVGOrMathMLElement.hpp"
#include "Krystal.Booey/SVG/Enums/SVGElementFlags.hpp"
#include "Krystal.Booey/SVG/Enums/SVGElementInterface.hpp"

namespace krys::boo::svg
{
  class SVGElement : public dom::StyledElement
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGElement);

  protected:
    SVGElementFlags _flags : BitCount<SVGElementFlags>() {SVGElementFlags::None};
    SVGElementInterface _interface : BitCount<SVGElementInterface>() {SVGElementInterface::None};

    SVGElement(dom::Document &document, SVGElementInterface interface,
               SVGElementFlags flags = SVGElementFlags::None,
               dom::NodeFlags nodeFlags = dom::NodeFlags::None) noexcept;

  public:
    SVGElement(dom::Document &document) noexcept;

#pragma region HTMLOrSVGOrMathMLElement Mixin - https://html.spec.whatwg.org/#htmlorsvgormathmlelement

    /// @see https://html.spec.whatwg.org/#dom-tabindex
    KRYS_NODISCARD int32 TabIndex() const noexcept
    {
      return html::mixins::HTMLOrSVGOrMathMLElement::TabIndex(*this);
    }

    /// @see https://html.spec.whatwg.org/#dom-tabindex
    void TabIndex(int32 value) noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::TabIndex(*this, value);
    }

    /// @see https://html.spec.whatwg.org/#dom-fe-autofocus
    KRYS_NODISCARD bool Autofocus() const noexcept
    {
      return html::mixins::HTMLOrSVGOrMathMLElement::Autofocus(*this);
    }

    /// @see https://html.spec.whatwg.org/#dom-fe-autofocus
    void Autofocus(bool value) noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::Autofocus(*this, value);
    }

    /// @see https://html.spec.whatwg.org/#dom-focus
    void Focus(const dom::FocusOptions &options = {}) noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::Focus(*this, options);
    }

    /// @see https://html.spec.whatwg.org/#dom-blur
    void Blur() noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::Blur(*this);
    }

#pragma endregion

    /// @brief Whether this is an `<svg>` element inside another SVG element.
    KRYS_NODISCARD bool IsInnerSVGSVGElement() const noexcept;

    /// @brief Whether this is an `<svg>` element that establishes an SVG document fragment: it has no parent
    /// node, its parent isn't an SVG element, or its parent is a `<foreignObject>`.
    KRYS_NODISCARD bool IsOutermostSVGSVGElement() const noexcept;

    /// @brief The property a presentation attribute maps to, or `PropertyId::Invalid` if the attribute isn't
    /// one.
    /// @see https://svgwg.org/svg2-draft/styling.html#PresentationAttributes
    KRYS_NODISCARD static css::PropertyId
      CssPropertyIdForSVGAttributeName(dom::DOMStringAtom localName,
                                       dom::DOMStringAtom namespaceURI) noexcept;

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;

#pragma region GlobalEventHandlers, DocumentAndElementEventHandlers - https://html.spec.whatwg.org/#event-handler-attributes

    KRYS_GLOBAL_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)
    KRYS_DOCUMENT_AND_ELEMENT_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)

#pragma endregion

#pragma region Type Checks

    KRYS_NODISCARD bool IsSVGCircleElement() const noexcept
    {
      return _interface == SVGElementInterface::Circle;
    }

    KRYS_NODISCARD bool IsSVGDefsElement() const noexcept
    {
      return _interface == SVGElementInterface::Defs;
    }

    KRYS_NODISCARD bool IsSVGDescElement() const noexcept
    {
      return _interface == SVGElementInterface::Desc;
    }

    KRYS_NODISCARD bool IsSVGEllipseElement() const noexcept
    {
      return _interface == SVGElementInterface::Ellipse;
    }

    KRYS_NODISCARD bool IsSVGForeignObjectElement() const noexcept
    {
      return _interface == SVGElementInterface::ForeignObject;
    }

    KRYS_NODISCARD bool IsSVGGElement() const noexcept
    {
      return _interface == SVGElementInterface::G;
    }

    KRYS_NODISCARD bool IsSVGGeometryElement() const noexcept
    {
      return HasFlag(_flags, SVGElementFlags::IsSvgGeometryElement);
    }

    KRYS_NODISCARD bool IsSVGGraphicsElement() const noexcept
    {
      return HasFlag(_flags, SVGElementFlags::IsSvgGraphicsElement);
    }

    KRYS_NODISCARD bool IsSVGImageElement() const noexcept
    {
      return _interface == SVGElementInterface::Image;
    }

    KRYS_NODISCARD bool IsSVGLineElement() const noexcept
    {
      return _interface == SVGElementInterface::Line;
    }

    KRYS_NODISCARD bool IsSVGMetadataElement() const noexcept
    {
      return _interface == SVGElementInterface::Metadata;
    }

    KRYS_NODISCARD bool IsSVGPolygonElement() const noexcept
    {
      return _interface == SVGElementInterface::Polygon;
    }

    KRYS_NODISCARD bool IsSVGPolylineElement() const noexcept
    {
      return _interface == SVGElementInterface::Polyline;
    }

    KRYS_NODISCARD bool IsSVGRectElement() const noexcept
    {
      return _interface == SVGElementInterface::Rect;
    }

    KRYS_NODISCARD bool IsSVGScriptElement() const noexcept
    {
      return _interface == SVGElementInterface::Script;
    }

    KRYS_NODISCARD bool IsSVGStyleElement() const noexcept
    {
      return _interface == SVGElementInterface::Style;
    }

    KRYS_NODISCARD bool IsSVGSvgElement() const noexcept
    {
      return _interface == SVGElementInterface::Svg;
    }

    KRYS_NODISCARD bool IsSVGSwitchElement() const noexcept
    {
      return _interface == SVGElementInterface::Switch;
    }

    KRYS_NODISCARD bool IsSVGSymbolElement() const noexcept
    {
      return _interface == SVGElementInterface::Symbol;
    }

    KRYS_NODISCARD bool IsSVGTitleElement() const noexcept
    {
      return _interface == SVGElementInterface::Title;
    }

    // NOTE: SVGUnknownElement type check is not needed here as Node already has it.

    KRYS_NODISCARD bool IsSVGUseElement() const noexcept
    {
      return _interface == SVGElementInterface::Use;
    }

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsSVGElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();