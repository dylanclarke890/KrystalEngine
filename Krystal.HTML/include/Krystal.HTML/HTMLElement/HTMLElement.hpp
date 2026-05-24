#pragma once

#include "Krystal.HTML/HTMLElement/HTMLTagName.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class Document;

  class HTMLElement : public Element
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLElement);

  protected:
    HTMLTagName _tagName {HTMLTagName::Unknown};

    HTMLElement(Document &document, HTMLTagName tag, NodeFlag flags = NodeFlag::None) noexcept;

  public:
    HTMLElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLElement)
  static bool IsType(const Krys::HTML::Node &target) noexcept
  {
    return target.IsHTMLElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();