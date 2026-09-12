#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/XMLDocument.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"

namespace krys::boo::svg
{
  class SVGDocument : public dom::XMLDocument
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGDocument);

  public:
    SVGDocument() noexcept : dom::XMLDocument(dom::DocumentFlags::IsSVGDocument)
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGDocument)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Document &document) noexcept
  {
    return document.IsSVGDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();