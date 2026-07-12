#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmldatalistelement
  class HTMLDataListElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDataListElement);

  public:
    HTMLDataListElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLDataListElement)
  static bool IsType(const Krys::HTML::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<Krys::HTML::HTMLElement>(target).IsHTMLDataListElement();
  }
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLDataListElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();