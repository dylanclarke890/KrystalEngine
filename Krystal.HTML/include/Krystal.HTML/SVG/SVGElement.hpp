#pragma once

#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/SVG/Enums/SVGTagName.hpp"

namespace Krys::HTML
{
  class SVGElement : public Element
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGElement);

  protected:
    SVGTagName _tagName {SVGTagName::Unknown};

    SVGElement(Document &document, SVGTagName tag, NodeFlags flags = NodeFlags::None) noexcept
        : Element(document, NullQualifiedName(), NodeFlags::IsSVGElement | flags), _tagName(tag)
    {
    }

  public:
    SVGElement(Document &document) noexcept : SVGElement(document, SVGTagName::Unknown)
    {
    }

  protected:
#pragma region Type Checks

    KRYS_NODISCARD bool IsSVGScriptElement() const noexcept
    {
      return _tagName == SVGTagName::Script;
    }

    // NOTE: SVGUnknownElement type check is not needed here as Node already has it.

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::SVGElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::Node &target) noexcept
  {
    return target.IsSVGElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();