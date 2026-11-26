#pragma once

#include "Krystal.Web/Dom/Nodes/XMLDocument.hpp"

namespace krys::web
{
  class XHTMLDocument : public XMLDocument
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(XHTMLDocument);

  public:
    XHTMLDocument() noexcept : XMLDocument(dom::DocumentFlag::IsXHTMLDocument)
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::XHTMLDocument)
  KRYS_NODISCARD static bool IsType(const krys::web::Document &document) noexcept
  {
    return document.IsXHTMLDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
