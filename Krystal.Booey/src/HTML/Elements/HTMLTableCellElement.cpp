#include "Krystal.Booey/HTML/Elements/HTMLTableCellElement.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableRowElement.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTableCell.hpp"

namespace krys::boo
{
  HTMLTableCellElement::HTMLTableCellElement(Document &document) noexcept
      : HTMLTablePartElement(document, html::HTMLElementInterface::TableCell)
  {
  }

#pragma region HTMLTableCellElement

  void HTMLTableCellElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                                Maybe<DOMStringView> value,
                                                const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLTablePartElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != null)
    {
      return;
    }

    if (localName == u8"rowspan" || localName == u8"colspan")
    {
      if (auto *tableCell = DynamicDowncast<RenderTableCell>(Renderer()))
      {
        tableCell->ColSpanOrRowSpanChanged();
      }
    }
  }

  /// @see https://html.spec.whatwg.org/#attr-th-scope
  template <>
  struct html::EnumeratedAttributeTraits<"scope", HTMLTableCellElement>
  {
    using State = html::ScopeAttributeState;
    static inline const html::KeywordStateMapping<State> StateMappings[] = {{u8"row", State::Row},
                                                                            {u8"col", State::Col},
                                                                            {u8"rowgroup", State::RowGroup},
                                                                            {u8"colgroup", State::ColGroup}};

    // The Auto state has no keyword.
    constexpr static State MissingValueDefault = State::Auto;
    constexpr static State InvalidValueDefault = State::Auto;
  };

  uint32 HTMLTableCellElement::ColSpan() const noexcept
  {
    return html::Reflect<uint32, html::ReflectDefault<uint32> {1}, html::ReflectRange<uint32> {1, 1'000}>(
      *this, u8"colspan");
  }

  void HTMLTableCellElement::ColSpan(uint32 value) noexcept
  {
    html::Reflect<uint32, html::ReflectDefault<uint32> {1}>(*this, u8"colspan", value);
  }

  uint32 HTMLTableCellElement::RowSpan() const noexcept
  {
    return html::Reflect<uint32, html::ReflectDefault<uint32> {1}, html::ReflectRange<uint32> {0, 65'534}>(
      *this, u8"rowspan");
  }

  void HTMLTableCellElement::RowSpan(uint32 value) noexcept
  {
    html::Reflect<uint32, html::ReflectDefault<uint32> {1}>(*this, u8"rowspan", value);
  }

  DOMString HTMLTableCellElement::Headers() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"headers").value_or(u8"");
  }

  void HTMLTableCellElement::Headers(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"headers", krys::move(value));
  }

  DOMString HTMLTableCellElement::Scope() const noexcept
  {
    return html::ReflectEnumerated<"scope", HTMLTableCellElement>(*this);
  }

  void HTMLTableCellElement::Scope(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"scope", krys::move(value));
  }

  DOMString HTMLTableCellElement::Abbr() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"abbr").value_or(u8"");
  }

  void HTMLTableCellElement::Abbr(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"abbr", krys::move(value));
  }

  int32 HTMLTableCellElement::CellIndex() const noexcept
  {
    if (!html::AsTableElement<HTMLTableRowElement>(ParentNode()))
    {
      return -1;
    }

    int32 index = 0;
    for (auto *sibling = PreviousSibling(); sibling; sibling = sibling->PreviousSibling())
    {
      if (html::AsTableElement<HTMLTableCellElement>(sibling))
      {
        ++index;
      }
    }
    return index;
  }

#pragma endregion

#pragma region HTMLTableCellElement Obsolete members

  DOMString HTMLTableCellElement::Align() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableCellElement::Align(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"align", krys::move(value));
  }

  DOMString HTMLTableCellElement::Axis() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"axis").value_or(u8"");
  }

  void HTMLTableCellElement::Axis(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"axis", krys::move(value));
  }

  DOMString HTMLTableCellElement::Height() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"height").value_or(u8"");
  }

  void HTMLTableCellElement::Height(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"height", krys::move(value));
  }

  DOMString HTMLTableCellElement::Width() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLTableCellElement::Width(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"width", krys::move(value));
  }

  DOMString HTMLTableCellElement::Char() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"char").value_or(u8"");
  }

  void HTMLTableCellElement::Char(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"char", krys::move(value));
  }

  DOMString HTMLTableCellElement::CharOff() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"charoff").value_or(u8"");
  }

  void HTMLTableCellElement::CharOff(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"charoff", krys::move(value));
  }

  bool HTMLTableCellElement::NoWrap() const noexcept
  {
    return html::Reflect<bool>(*this, u8"nowrap");
  }

  void HTMLTableCellElement::NoWrap(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"nowrap", krys::move(value));
  }

  DOMString HTMLTableCellElement::VAlign() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"valign").value_or(u8"");
  }

  void HTMLTableCellElement::VAlign(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"valign", krys::move(value));
  }

  DOMString HTMLTableCellElement::BgColor() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"bgcolor").value_or(u8"");
  }

  void HTMLTableCellElement::BgColor(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"bgcolor", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool
    HTMLTableCellElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                             const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null
        && (localName == u8"nowrap" || localName == u8"width" || localName == u8"height"))
    {
      return true;
    }
    return HTMLTablePartElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLTableCellElement::CollectPresentationalHintsForAttribute(
    const DOMStringAtom &localName, const DOMStringAtom &namespaceURI, DOMStringView value,
    css::MutableStyleProperties &style) noexcept
  {
    using namespace html::presentational_hints;
    if (namespaceURI != null)
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
    // Both map to their dimension properties, ignoring zero.
    // https://html.spec.whatwg.org/#tables-2
    if (localName == u8"width")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::Width, value, AllowZeroValue::No);
      return;
    }
    if (localName == u8"height")
    {
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
