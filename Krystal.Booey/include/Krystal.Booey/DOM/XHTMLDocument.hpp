#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/XMLDocument.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"

namespace krys::boo::dom
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::XHTMLDocument)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Document &document) noexcept
  {
    return document.IsXHTMLDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();