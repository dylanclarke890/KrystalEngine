#pragma once

#include "Krystal.HTML/HTML/HTMLMediaElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlaudioelement
  class HTMLAudioElement : public HTMLMediaElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLAudioElement);

  public:
    HTMLAudioElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLAudioElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLAudioElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();