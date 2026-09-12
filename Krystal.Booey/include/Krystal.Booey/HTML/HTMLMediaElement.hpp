#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlmediaelement
  class HTMLMediaElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMediaElement);

  protected:
    HTMLMediaElement(Document &document, HTMLElementInterface interface) noexcept;

  public:
    HTMLMediaElement(Document &document) noexcept;

    // TODO(HTMLMediaElement, HTML): Implement the media element API.
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLMediaElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLMediaElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();