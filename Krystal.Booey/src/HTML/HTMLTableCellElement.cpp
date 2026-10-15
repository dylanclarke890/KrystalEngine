#include "Krystal.Booey/HTML/HTMLTableCellElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo::html
{
  HTMLTableCellElement::HTMLTableCellElement(dom::Document &document) noexcept
      : HTMLTablePartElement(document, HTMLElementInterface::TableCell)
  {
  }

#pragma region HTMLTableCellElement

  uint32 HTMLTableCellElement::ColSpan() const noexcept
  {
    return Reflection::Reflect<uint32, ReflectDefault<uint32> {1}, ReflectRange<uint32> {1, 1'000}>(
      *this, u8"colspan");
  }

  void HTMLTableCellElement::ColSpan(uint32 value) noexcept
  {
    Reflection::Reflect<uint32, ReflectDefault<uint32> {1}>(*this, u8"colspan", value);
  }

  uint32 HTMLTableCellElement::RowSpan() const noexcept
  {
    return Reflection::Reflect<uint32, ReflectDefault<uint32> {1}, ReflectRange<uint32> {1, 65'534}>(
      *this, u8"rowspan");
  }

  void HTMLTableCellElement::RowSpan(uint32 value) noexcept
  {
    Reflection::Reflect<uint32, ReflectDefault<uint32> {1}>(*this, u8"rowspan", value);
  }

  dom::DOMString HTMLTableCellElement::Headers() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"headers").value_or(u8"");
  }

  void HTMLTableCellElement::Headers(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"headers", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::Scope() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"scope").value_or(u8"");
  }

  void HTMLTableCellElement::Scope(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"scope", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::Abbr() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"abbr").value_or(u8"");
  }

  void HTMLTableCellElement::Abbr(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"abbr", krys::move(value));
  }

#pragma endregion

#pragma region HTMLTableCellElement Obsolete members

  dom::DOMString HTMLTableCellElement::Align() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableCellElement::Align(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::Axis() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"axis").value_or(u8"");
  }

  void HTMLTableCellElement::Axis(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"axis", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::Height() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"height").value_or(u8"");
  }

  void HTMLTableCellElement::Height(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"height", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::Width() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLTableCellElement::Width(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"width", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::Char() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"char").value_or(u8"");
  }

  void HTMLTableCellElement::Char(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"char", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::CharOff() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"charoff").value_or(u8"");
  }

  void HTMLTableCellElement::CharOff(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"charoff", krys::move(value));
  }

  bool HTMLTableCellElement::NoWrap() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"nowrap");
  }

  void HTMLTableCellElement::NoWrap(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"nowrap", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::VAlign() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"valign").value_or(u8"");
  }

  void HTMLTableCellElement::VAlign(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"valign", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::BgColor() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"bgcolor").value_or(u8"");
  }

  void HTMLTableCellElement::BgColor(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"bgcolor", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool
    HTMLTableCellElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                             dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null()
        && (localName == u8"nowrap" || localName == u8"width" || localName == u8"height"))
    {
      return true;
    }
    return HTMLTablePartElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLTableCellElement::CollectPresentationalHintsForAttribute(
    dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI, dom::DOMStringView value,
    css::MutableStyleProperties &style) noexcept
  {
    using namespace presentational_hints;
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      HTMLTablePartElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"nowrap")
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::WhiteSpaceCollapse,
                                           css::ValueId::Collapse);
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextWrapMode, css::ValueId::Nowrap);
      return;
    }
    if (localName == u8"width")
    {
      // width="0" is not allowed for compatibility with WinIE.
      AddHTMLLengthToStyle(style, css::PropertyId::Width, value, AllowZeroValue::No);
      return;
    }
    if (localName == u8"height")
    {
      // width="0" is not allowed for compatibility with WinIE.
      AddHTMLLengthToStyle(style, css::PropertyId::Height, value, AllowZeroValue::No);
      return;
    }
    HTMLTablePartElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

  const css::MutableStyleProperties *HTMLTableCellElement::AdditionalPresentationalHintStyle() const noexcept
  {
    if (const auto *table = FindParentTable())
    {
      return table->AdditionalCellStyle();
    }
    return nullptr;
  }

#pragma endregion
}
