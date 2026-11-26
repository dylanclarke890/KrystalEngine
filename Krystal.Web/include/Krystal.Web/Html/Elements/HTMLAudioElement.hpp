#pragma once

#include "Krystal.Web/Html/Elements/HTMLMediaElement.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/media.html#htmlaudioelement
  class HTMLAudioElement : public HTMLMediaElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLAudioElement);

  public:
    HTMLAudioElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLAudioElement)
  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLAudioElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
