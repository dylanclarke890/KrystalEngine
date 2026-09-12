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
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::HTMLDocument)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Document &document) noexcept
  {
    return document.IsHTMLDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
