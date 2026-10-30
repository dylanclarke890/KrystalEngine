#pragma once

#include "Krystal.Booey/DOM/Document.hpp"

namespace krys::boo::dom
{
  class HTMLDocument : public Document
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDocument);

  public:
    HTMLDocument() noexcept : Document(DocumentFlags::IsHTMLDocument)
    {
    }

    /// @brief Whether attribute selectors match the values of @p attributeName case-sensitively on HTML
    /// elements in HTML documents.
    /// @see https://html.spec.whatwg.org/#case-sensitivity-of-selectors
    KRYS_NODISCARD static bool IsCaseSensitiveAttribute(const QualifiedName &attributeName) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::HTMLDocument)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Document &document) noexcept
  {
    return document.IsHTMLDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
