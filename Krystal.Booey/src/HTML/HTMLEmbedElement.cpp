#include "Krystal.Booey/HTML/HTMLEmbedElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo::html
{
#pragma region HTMLEmbedElement

  HTMLEmbedElement::HTMLEmbedElement(dom::Document &document) noexcept
      : HTMLPlugInElement(document, HTMLElementInterface::Embed)
  {
  }

  dom::ExceptionOr<dom::USVString> HTMLEmbedElement::Src() const noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"src");
  }

  dom::ExceptionOr<void> HTMLEmbedElement::Src(dom::USVString &&value) noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"src", krys::move(value));
  }

  dom::DOMString HTMLEmbedElement::Type() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLEmbedElement::Type(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

  dom::DOMString HTMLEmbedElement::Width() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLEmbedElement::Width(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"width", krys::move(value));
  }

  dom::DOMString HTMLEmbedElement::Height() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"height").value_or(u8"");
  }

  void HTMLEmbedElement::Height(dom::DOMString value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"height", krys::move(value));
  }

#pragma endregion

#pragma region HTMLEmbedElement Obsolete members

  dom::DOMString HTMLEmbedElement::Align() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLEmbedElement::Align(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  dom::DOMString HTMLEmbedElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLEmbedElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  void HTMLEmbedElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                                dom::DOMStringAtom namespaceURI,
                                                                dom::DOMStringView value,
                                                                css::MutableStyleProperties &style) noexcept
  {
    using namespace presentational_hints;
    if (namespaceURI != dom::DOMStringAtom::Null())
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
