#pragma once

#include "Krystal.Booey/DOM/Nodes/Document.hpp"

namespace krys::boo
{
  class XMLDocument : public Document
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(XMLDocument);

  protected:
    XMLDocument(dom::DocumentFlags flags) noexcept : Document(flags | dom::DocumentFlags::IsXMLDocument)
    {
    }

  public:
    XMLDocument() noexcept : Document(dom::DocumentFlags::IsXMLDocument)
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
