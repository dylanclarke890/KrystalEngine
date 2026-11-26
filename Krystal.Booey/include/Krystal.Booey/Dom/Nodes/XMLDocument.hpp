#pragma once

#include "Krystal.Booey/Dom/Nodes/Document.hpp"

namespace krys::boo
{
  class XMLDocument : public Document
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(XMLDocument);

  protected:
    XMLDocument(dom::DocumentFlags flags) noexcept : Document(flags | dom::DocumentFlag::IsXMLDocument)
    {
    }

  public:
    XMLDocument() noexcept : Document(dom::DocumentFlag::IsXMLDocument)
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::XMLDocument)
  KRYS_NODISCARD static bool IsType(const krys::boo::Document &document) noexcept
  {
    return document.IsXMLDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
