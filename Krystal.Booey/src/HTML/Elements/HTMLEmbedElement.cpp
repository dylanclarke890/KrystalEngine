#include "Krystal.Booey/HTML/Elements/HTMLEmbedElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo
{
#pragma region HTMLEmbedElement

  HTMLEmbedElement::HTMLEmbedElement(Document &document) noexcept
      : HTMLPlugInElement(document, html::HTMLElementInterface::Embed)
  {
  }

  dom::USVString HTMLEmbedElement::Src() const noexcept
  {
    return html::ReflectURL<dom::USVString>(*this, u8"src");
  }

  void HTMLEmbedElement::Src(dom::USVString value) noexcept
  {
    return html::ReflectURL<dom::USVString>(*this, u8"src", krys::move(value));
  }

  DOMString HTMLEmbedElement::Type() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLEmbedElement::Type(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"type", krys::move(value));
  }

  DOMString HTMLEmbedElement::Width() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLEmbedElement::Width(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"width", krys::move(value));
  }

  DOMString HTMLEmbedElement::Height() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"height").value_or(u8"");
  }

  void HTMLEmbedElement::Height(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"height", krys::move(value));
  }

#pragma endregion

#pragma region HTMLEmbedElement Obsolete members

  DOMString HTMLEmbedElement::Align() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLEmbedElement::Align(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"align", krys::move(value));
  }

  DOMString HTMLEmbedElement::Name() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLEmbedElement::Name(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"name", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  void HTMLEmbedElement::CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                                const DOMStringAtom &namespaceURI,
                                                                DOMStringView value,
                                                                css::MutableStyleProperties &style) noexcept
  {
    using namespace html::presentational_hints;
    if (namespaceURI != null)
    {
      HTMLPlugInElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"hidden")
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::Width, 0, css::UnitType::px);
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::Height, 0, css::UnitType::px);
      return;
    }
    HTMLPlugInElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
