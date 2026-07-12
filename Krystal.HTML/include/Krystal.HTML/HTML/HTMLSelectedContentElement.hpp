#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlselectedcontentelement
  class HTMLSelectedContentElement : public HTMLElement
  {
    friend class FormControlAlgorithms;

    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSelectedContentElement);

  public:
    HTMLSelectedContentElement(Document &document) noexcept;

    // TODO(HTMLSelectedContentElement): implement disabled properly
    KRYS_NODISCARD bool Disabled() const noexcept
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLSelectedContentElement)
  static bool IsType(const Krys::HTML::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<Krys::HTML::HTMLElement>(target).IsHTMLSelectedContentElement();
  }
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLSelectedContentElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
