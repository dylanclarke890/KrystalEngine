#pragma once

#include "Krystal.HTML/DOM/Document.hpp"

namespace Krys::HTML
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLDocument)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::Document &document) noexcept
  {
    return document.IsHTMLDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
