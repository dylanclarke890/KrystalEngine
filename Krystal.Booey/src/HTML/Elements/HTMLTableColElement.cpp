#include "Krystal.Booey/HTML/Elements/HTMLTableColElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTableCol.hpp"

namespace krys::boo
{
  HTMLTableColElement::HTMLTableColElement(Document &document) noexcept
      : HTMLTablePartElement(document, html::HTMLElementInterface::TableCol)
  {
  }

#pragma region HTMLTableColElement

  void HTMLTableColElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                               Maybe<DOMStringView> value,
                                               const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLTablePartElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != null)
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
    // A width change needs nothing here: it's a presentational hint, so the column is styled and laid out
    // again.
  }

  uint32 HTMLTableColElement::Span() const noexcept
  {
    return html::Reflect<uint32, html::ReflectDefault<uint32> {1}, html::ReflectRange<uint32> {1u, 1'000u}>(
      *this, u8"span");
  }

  void HTMLTableColElement::Span(uint32 value) noexcept
  {
    html::Reflect<uint32, html::ReflectDefault<uint32> {1}>(*this, u8"span", krys::move(value));
  }

#pragma endregion

#pragma region HTMLTableColElement Obsolete members

  DOMString HTMLTableColElement::Align() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableColElement::Align(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"align", krys::move(value));
  }

  DOMString HTMLTableColElement::Char() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"char").value_or(u8"");
  }

  void HTMLTableColElement::Char(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"char", krys::move(value));
  }

  DOMString HTMLTableColElement::CharOff() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"charoff").value_or(u8"");
  }

  void HTMLTableColElement::CharOff(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"charoff", krys::move(value));
  }

  DOMString HTMLTableColElement::VAlign() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"valign").value_or(u8"");
  }

  void HTMLTableColElement::VAlign(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"valign", krys::move(value));
  }

  DOMString HTMLTableColElement::Width() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLTableColElement::Width(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"width", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool
    HTMLTableColElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                            const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null && (localName == u8"width"))
    {
      return true;
    }
    return HTMLTablePartElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLTableColElement::CollectPresentationalHintsForAttribute(
    const DOMStringAtom &localName, const DOMStringAtom &namespaceURI, DOMStringView value,
    css::MutableStyleProperties &style) noexcept
  {
    using namespace html::presentational_hints;
    if (namespaceURI != null)
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
    if (TagQualifiedName().TagName() != html::TagName::colgroup)
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
