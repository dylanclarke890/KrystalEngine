#pragma once

#include "Krystal.Booey/DOM/Nodes/XMLDocument.hpp"

namespace krys::boo
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::XHTMLDocument)
  KRYS_NODISCARD static bool IsType(const krys::boo::Document &document) noexcept
  {
    return document.IsXHTMLDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
