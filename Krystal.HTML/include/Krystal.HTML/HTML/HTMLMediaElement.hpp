#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlmediaelement
  class HTMLMediaElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMediaElement);

  protected:
    HTMLMediaElement(Document &document, DOMInterface interface) noexcept;

  public:
    HTMLMediaElement(Document &document) noexcept;

    // TODO(HTMLMediaElement, HTML): Implement the media element API.
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLMediaElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLMediaElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();