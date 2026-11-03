#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @brief The obsolete `dir` element.
  /// @see https://html.spec.whatwg.org/#htmldirectoryelement
  class HTMLDirectoryElement final : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDirectoryElement);

  public:
    explicit HTMLDirectoryElement(dom::Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dir-compact
    KRYS_NODISCARD bool Compact() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dir-compact
    void Compact(bool value) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLDirectoryElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLDirectoryElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
