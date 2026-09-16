#pragma once

#include "Krystal.Booey/HTML/HTMLMediaElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlaudioelement
  class HTMLAudioElement : public HTMLMediaElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLAudioElement);

  public:
    HTMLAudioElement(dom::Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLAudioElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLAudioElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();