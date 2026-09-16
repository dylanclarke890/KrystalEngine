#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlselectedcontentelement
  class HTMLSelectedContentElement : public HTMLElement
  {
    friend class FormControlAlgorithms;

    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSelectedContentElement);

  public:
    HTMLSelectedContentElement(dom::Document &document) noexcept;

    // TODO(HTMLSelectedContentElement): implement disabled properly
    KRYS_NODISCARD bool Disabled() const noexcept
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLSelectedContentElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement()
           && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLSelectedContentElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLSelectedContentElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
