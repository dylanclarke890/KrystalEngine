#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/XMLDocument.hpp"

namespace Krys::HTML
{
  class XHTMLDocument : public XMLDocument
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(XHTMLDocument);

  public:
    XHTMLDocument() noexcept : XMLDocument(DocumentFlags::IsXHTMLDocument)
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::XHTMLDocument)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::Document &document) noexcept
  {
    return document.IsXHTMLDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();