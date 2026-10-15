#include "Krystal.Booey/HTML/HTMLPreElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo::html
{
  HTMLPreElement::HTMLPreElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Pre)
  {
  }

#pragma region HTMLPreElement Obsolete members

  int32 HTMLPreElement::Width() const noexcept
  {
    return Reflection::Reflect<int32>(*this, u8"width");
  }

  void HTMLPreElement::Width(int32 value) noexcept
  {
    Reflection::Reflect<int32>(*this, u8"width", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLPreElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                          dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null() && (localName == u8"wrap"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLPreElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                              dom::DOMStringAtom namespaceURI,
                                                              dom::DOMStringView value,
                                                              css::MutableStyleProperties &style) noexcept
  {
    using namespace presentational_hints;
    if (namespaceURI != dom::DOMStringAtom::Null())
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
