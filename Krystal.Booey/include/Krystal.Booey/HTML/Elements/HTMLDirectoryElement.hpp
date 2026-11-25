#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @brief The obsolete `dir` element.
  /// @see https://html.spec.whatwg.org/multipage/obsolete.html#htmldirectoryelement
  class HTMLDirectoryElement final : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDirectoryElement);

  public:
    explicit HTMLDirectoryElement(Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-dir-compact
    KRYS_NODISCARD bool Compact() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-dir-compact
    void Compact(bool value) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLDirectoryElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLDirectoryElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
