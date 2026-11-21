#include "Krystal.Booey/HTML/Elements/HTMLPreElement.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo
{
  HTMLPreElement::HTMLPreElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Pre)
  {
  }

#pragma region HTMLPreElement Obsolete members

  int32 HTMLPreElement::Width() const noexcept
  {
    return html::Reflect<int32>(*this, u8"width");
  }

  void HTMLPreElement::Width(int32 value) noexcept
  {
    html::Reflect<int32>(*this, u8"width", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLPreElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                          const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null && (localName == u8"wrap"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLPreElement::CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                              const DOMStringAtom &namespaceURI,
                                                              DOMStringView value,
                                                              css::MutableStyleProperties &style) noexcept
  {
    using namespace html::presentational_hints;
    if (namespaceURI != null)
    {
      HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"wrap")
    {
      style.SetProperty(css::PropertyId::WhiteSpaceCollapse, css::ValueId::Preserve);
      style.SetProperty(css::PropertyId::TextWrapMode, css::ValueId::Wrap);
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
