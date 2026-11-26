#pragma once

#include "Krystal.Booey/Dom/Nodes/Document.hpp"

namespace krys::boo
{
  class QualifiedName;

  /// @brief A document whose type is "html", with the content type text/html.
  /// @see https://dom.spec.whatwg.org/#html-document
  class HTMLDocument : public Document
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDocument);

  public:
    HTMLDocument() noexcept : Document(dom::DocumentFlag::IsHTMLDocument)
    {
    }

    /// @brief An empty HTML document with @p flags as well.
    explicit HTMLDocument(dom::DocumentFlags flags) noexcept
        : Document(flags | dom::DocumentFlag::IsHTMLDocument)
    {
    }

    /// @brief Whether attribute selectors match the values of @p attributeName case-sensitively on HTML
    /// elements in HTML documents.
    /// @see https://html.spec.whatwg.org/multipage/semantics-other.html#case-sensitivity-of-selectors
    KRYS_NODISCARD static bool IsCaseSensitiveAttribute(const QualifiedName &attributeName) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLDocument)
  KRYS_NODISCARD static bool IsType(const krys::boo::Document &document) noexcept
  {
    return document.IsHTMLDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
