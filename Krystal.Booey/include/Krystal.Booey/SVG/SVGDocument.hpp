#pragma once

#include "Krystal.Booey/DOM/Nodes/XMLDocument.hpp"

namespace krys::boo
{
  class SVGDocument : public XMLDocument
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGDocument);

  public:
    SVGDocument() noexcept : XMLDocument(dom::DocumentFlag::IsSVGDocument)
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SVGDocument)
  KRYS_NODISCARD static bool IsType(const krys::boo::Document &document) noexcept
  {
    return document.IsSVGDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
