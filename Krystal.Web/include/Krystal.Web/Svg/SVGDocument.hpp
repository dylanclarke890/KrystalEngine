#pragma once

#include "Krystal.Web/Dom/Nodes/XMLDocument.hpp"

namespace krys::web
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::SVGDocument)
  KRYS_NODISCARD static bool IsType(const krys::web::Document &document) noexcept
  {
    return document.IsSVGDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
