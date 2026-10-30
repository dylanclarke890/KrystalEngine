#include "Krystal.Booey/HTML/HTMLTableColElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/Rendering/RenderTableCol.hpp"

namespace krys::boo::html
{
  HTMLTableColElement::HTMLTableColElement(dom::Document &document) noexcept
      : HTMLTablePartElement(document, HTMLElementInterface::TableCol)
  {
  }

#pragma region HTMLTableColElement

  void HTMLTableColElement::OnAttributeChanged(dom::DOMStringAtom localName,
                                               Maybe<dom::DOMStringView> oldValue,
                                               Maybe<dom::DOMStringView> value,
                                               dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLTablePartElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    if (localName == u8"span")
    {
      if (auto *col = DynamicDowncast<RenderTableCol>(Renderer()))
      {
        col->UpdateFromElement();
      }
    }
    // FIXME: Lay the column out again when its width attribute changes to a different integer
    // (parseHTMLInteger, which Numbers::ParseInteger provides).
  }

  uint32 HTMLTableColElement::Span() const noexcept
  {
    return Reflection::Reflect<uint32, ReflectDefault<uint32> {1}, ReflectRange<uint32> {1u, 1'000u}>(
      *this, u8"span");
  }

  void HTMLTableColElement::Span(uint32 value) noexcept
  {
    Reflection::Reflect<uint32, ReflectDefault<uint32> {1}>(*this, u8"span", krys::move(value));
  }

#pragma endregion

#pragma region HTMLTableColElement Obsolete members

  dom::DOMString HTMLTableColElement::Align() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableColElement::Align(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  dom::DOMString HTMLTableColElement::Char() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"char").value_or(u8"");
  }

  void HTMLTableColElement::Char(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"char", krys::move(value));
  }

  dom::DOMString HTMLTableColElement::CharOff() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"charoff").value_or(u8"");
  }

  void HTMLTableColElement::CharOff(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"charoff", krys::move(value));
  }

  dom::DOMString HTMLTableColElement::VAlign() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"valign").value_or(u8"");
  }

  void HTMLTableColElement::VAlign(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"valign", krys::move(value));
  }

  dom::DOMString HTMLTableColElement::Width() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLTableColElement::Width(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"width", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLTableColElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                               dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null() && (localName == u8"width"))
    {
      return true;
    }
    return HTMLTablePartElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLTableColElement::CollectPresentationalHintsForAttribute(
    dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI, dom::DOMStringView value,
    css::MutableStyleProperties &style) noexcept
  {
    using namespace presentational_hints;
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      HTMLTablePartElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"width")
    {
      AddHTMLMultiLengthToStyle(style, css::PropertyId::Width, value);
      return;
    }
    if (localName == u8"height")
    {
      AddHTMLMultiLengthToStyle(style, css::PropertyId::Height, value);
      return;
    }
    HTMLTablePartElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

  const css::MutableStyleProperties *HTMLTableColElement::AdditionalPresentationalHintStyle() const noexcept
  {
    if (TagQualifiedName().TagName() != TagName::colgroup)
    {
      return nullptr;
    }
    if (const auto *table = FindParentTable())
    {
      return table->AdditionalGroupStyle(false);
    }
    return nullptr;
  }

#pragma endregion
}
