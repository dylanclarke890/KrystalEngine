#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/XMLDocument.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"

namespace Krys::HTML
{
  class SVGDocument : public XMLDocument
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGDocument);

  public:
    SVGDocument() noexcept : XMLDocument(DocumentFlags::IsSVGDocument)
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::SVGDocument)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::Document &document) noexcept
  {
    return document.IsSVGDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();