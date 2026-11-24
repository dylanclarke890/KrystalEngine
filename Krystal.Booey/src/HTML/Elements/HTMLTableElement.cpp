/*
 * Copyright (C) 1997 Martin Jones (mjones@kde.org)
 *           (C) 1997 Torben Weis (weis@kde.org)
 *           (C) 1998 Waldo Bastian (bastian@kde.org)
 *           (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2004, 2005, 2006, 2008, 2010, 2011 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/HTML/Elements/HTMLTableElement.hpp"
#include "algorithm"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Collections/HTMLTableRowsCollection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableCaptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableRowElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableSectionElement.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Core/Memory/NeverDestroyed.hpp"

namespace krys::boo
{
  HTMLTableElement::HTMLTableElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Table)
  {
  }

  HTMLTableElement::~HTMLTableElement() noexcept = default;

#pragma region HTMLTableElement

  namespace
  {
    KRYS_NODISCARD bool IsTableChildWithTagName(const Node &node, html::TagName tagName) noexcept
    {
      auto *element = DynamicDowncast<HTMLElement>(node);
      return element && element->HasTagName(tagName);
    }

    // The first child of the table that is a section with the tag name.
    KRYS_NODISCARD RefPtr<HTMLTableSectionElement> FirstTableSection(const HTMLTableElement &table,
                                                                     html::TagName tagName) noexcept
    {
      for (auto *child = table.FirstChild(); child; child = child->NextSibling())
      {
        auto *section = html::AsTableElement<HTMLTableSectionElement>(child);
        if (section && section->HasTagName(tagName))
        {
          return ShareRefPtr(const_cast<HTMLTableSectionElement *>(section));
        }
      }
      return nullptr;
    }
  }

  RefPtr<HTMLTableCaptionElement> HTMLTableElement::Caption() const noexcept
  {
    for (auto *child = FirstChild(); child; child = child->NextSibling())
    {
      if (auto *caption = html::AsTableElement<HTMLTableCaptionElement>(child))
      {
        return ShareRefPtr(const_cast<HTMLTableCaptionElement *>(caption));
      }
    }
    return nullptr;
  }

  ExceptionOr<void> HTMLTableElement::Caption(const RefPtr<HTMLTableCaptionElement> &caption) noexcept
  {
    html::CEReactionsScope ceReactions;

    // 1. Remove the first caption element child of this, if any.
    DeleteCaption();

    // 2. If the given value is not null, then insert it as the first node of this.
    if (!caption)
    {
      return {};
    }
    if (auto result = InsertBefore(*caption, FirstChild()); result.HasException())
    {
      return result.ReleaseException();
    }
    return {};
  }

  Ref<HTMLTableCaptionElement> HTMLTableElement::CreateCaption() noexcept
  {
    // 1. If this has a caption element child, then return the first such element.
    if (auto existingCaption = Caption())
    {
      return krys::move(existingCaption);
    }

    // 2. Let caption be the result of creating a table element given this and "caption".
    auto caption = html::CreateTableElement<HTMLTableCaptionElement>(NodeDocument(), u8"caption");

    // 3. Insert caption as the first node of this.
    (void)Caption(RefPtr<HTMLTableCaptionElement>(caption));

    // 4. Return caption.
    return caption;
  }

  void HTMLTableElement::DeleteCaption() noexcept
  {
    html::CEReactionsScope ceReactions;
    if (auto caption = Caption())
    {
      (void)RemoveChild(*caption);
    }
  }

  RefPtr<HTMLTableSectionElement> HTMLTableElement::THead() const noexcept
  {
    return FirstTableSection(*this, html::TagName::thead);
  }

  ExceptionOr<void> HTMLTableElement::THead(const RefPtr<HTMLTableSectionElement> &head) noexcept
  {
    html::CEReactionsScope ceReactions;

    // 1. If the given value is neither null nor a thead element, then throw a "HierarchyRequestError"
    //    DOMException.
    if (head && !head->HasTagName(html::TagName::thead)) KRYS_UNLIKELY
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    // 2. Remove the first thead element child of this, if any.
    DeleteTHead();

    // 3. If the given value is not null, then insert it immediately before the first element child of this
    //    that is neither a caption element nor a colgroup element, if any; otherwise insert it at the end of
    //    this.
    if (!head)
    {
      return {};
    }
    Node *child = FirstChild();
    for (; child; child = child->NextSibling())
    {
      if (Is<Element>(*child) && !IsTableChildWithTagName(*child, html::TagName::caption)
          && !IsTableChildWithTagName(*child, html::TagName::colgroup))
      {
        break;
      }
    }

    if (auto result = InsertBefore(*head, child); result.HasException())
    {
      return result.ReleaseException();
    }
    return {};
  }

  Ref<HTMLTableSectionElement> HTMLTableElement::CreateTHead() noexcept
  {
    // 1. If this has a thead element child, then return the first such element.
    if (auto existingHead = THead())
    {
      return krys::move(existingHead);
    }

    // 2. Let thead be the result of creating a table element given this and "thead".
    auto head = html::CreateTableElement<HTMLTableSectionElement>(NodeDocument(), u8"thead");

    // 3. Insert thead immediately before the first element child of this that is neither a caption element
    // nor
    //    a colgroup element, if any; otherwise insert thead at the end of this.
    (void)THead(RefPtr<HTMLTableSectionElement>(head));

    // 4. Return thead.
    return head;
  }

  void HTMLTableElement::DeleteTHead() noexcept
  {
    html::CEReactionsScope ceReactions;
    if (auto head = THead())
    {
      (void)RemoveChild(*head);
    }
  }

  RefPtr<HTMLTableSectionElement> HTMLTableElement::TFoot() const noexcept
  {
    return FirstTableSection(*this, html::TagName::tfoot);
  }

  ExceptionOr<void> HTMLTableElement::TFoot(const RefPtr<HTMLTableSectionElement> &foot) noexcept
  {
    html::CEReactionsScope ceReactions;

    // 1. If the given value is neither null nor a tfoot element, then throw a "HierarchyRequestError"
    //    DOMException.
    if (foot && !foot->HasTagName(html::TagName::tfoot)) KRYS_UNLIKELY
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    // 2. Remove the first tfoot element child of this, if any.
    DeleteTFoot();

    // 3. If the given value is not null, then insert it at the end of this.
    if (!foot)
    {
      return {};
    }

    if (auto result = AppendChild(*foot); result.HasException())
    {
      return result.ReleaseException();
    }
    return {};
  }

  Ref<HTMLTableSectionElement> HTMLTableElement::CreateTFoot() noexcept
  {
    // 1. If this has a tfoot element child, then return the first such element.
    if (auto existingFoot = TFoot())
    {
      return krys::move(existingFoot);
    }

    // 2. Let tfoot be the result of creating a table element given this and "tfoot".
    auto foot = html::CreateTableElement<HTMLTableSectionElement>(NodeDocument(), u8"tfoot");

    // 3. Insert tfoot at the end of this.
    (void)TFoot(RefPtr<HTMLTableSectionElement>(foot));

    // 4. Return tfoot.
    return foot;
  }

  void HTMLTableElement::DeleteTFoot() noexcept
  {
    html::CEReactionsScope ceReactions;
    if (auto foot = TFoot())
    {
      (void)RemoveChild(*foot);
    }
  }

  Ref<dom::HTMLCollection> HTMLTableElement::TBodies() noexcept
  {
    if (auto *tBodies = _tBodies.get())
    {
      return ShareRef(*tBodies);
    }

    Ref<dom::HTMLCollection> tBodies = CreateRef<dom::LiveHTMLCollection>(
      *this, [](const Element &element) { return IsTableChildWithTagName(element, html::TagName::tbody); },
      dom::LiveCollectionInvalidation::TreeOnly, dom::LiveCollectionTraversal::Children);
    _tBodies = CreateWeakPtr(tBodies.get());
    return tBodies;
  }

  Ref<HTMLTableSectionElement> HTMLTableElement::CreateTBody() noexcept
  {
    // 1. Let tbody be the result of creating a table element given this and "tbody".
    auto body = html::CreateTableElement<HTMLTableSectionElement>(NodeDocument(), u8"tbody");

    // 2. Insert tbody immediately after the last tbody element child of this, if any; otherwise insert tbody
    // at
    //    the end of this.
    auto *lastBody = LastBody();
    (void)InsertBefore(*body, lastBody ? lastBody->NextSibling() : nullptr);

    // 3. Return tbody.
    return body;
  }

  HTMLTableSectionElement *HTMLTableElement::LastBody() const noexcept
  {
    for (auto *child = LastChild(); child; child = child->PreviousSibling())
    {
      if (IsTableChildWithTagName(*child, html::TagName::tbody))
      {
        return const_cast<HTMLTableSectionElement *>(html::AsTableElement<HTMLTableSectionElement>(child));
      }
    }
    return nullptr;
  }

  Ref<dom::HTMLCollection> HTMLTableElement::Rows() noexcept
  {
    if (auto *rows = _rows.get())
    {
      return ShareRef(*rows);
    }

    Ref<dom::HTMLCollection> rows = CreateRef<html::HTMLTableRowsCollection>(*this);
    _rows = CreateWeakPtr(rows.get());
    return rows;
  }

  ExceptionOr<Ref<HTMLTableRowElement>> HTMLTableElement::InsertRow(int32 index) noexcept
  {
    // 1. If index is less than −1 or greater than the number of elements in the rows collection, then throw
    // an
    //    "IndexSizeError" DOMException.
    // (Greater is found while walking the rows.)
    if (index < -1)
    {
      return Exception {ExceptionCode::IndexSizeError};
    }

    Ref<HTMLTableElement> protectedThis = ShareRef(*this);

    RefPtr<HTMLTableRowElement> lastRow;
    RefPtr<HTMLTableRowElement> row;
    if (index == -1)
    {
      lastRow = ShareRefPtr(html::HTMLTableRowsCollection::LastRow(*this));
    }
    else
    {
      for (int32 i = 0; i <= index; ++i)
      {
        row = ShareRefPtr(html::HTMLTableRowsCollection::RowAfter(*this, lastRow.get()));
        if (!row)
        {
          if (i != index)
          {
            return Exception {ExceptionCode::IndexSizeError};
          }
          break;
        }
        lastRow = row;
      }
    }

    // 2. Let tr be the result of creating a table element given this and "tr".
    auto &document = NodeDocument();
    auto newRow = html::CreateTableElement<HTMLTableRowElement>(document, u8"tr");

    RefPtr<ContainerNode> parent;
    // 5. Otherwise, if index is −1 or equal to the number of items in the rows collection, then append tr to
    //    the parent of the last tr element in the rows collection.
    // 6. Otherwise, insert tr immediately before the indexth tr element in the rows collection, in the same
    //    parent.
    if (lastRow)
    {
      parent = ShareRefPtr(row ? row->ParentNode() : lastRow->ParentNode());
    }
    else
    {
      // 4. Otherwise, if the rows collection has zero elements in it, then append tr to the last tbody
      // element
      //    in this.
      // (The last tbody child, as in browsers.)
      parent = ShareRefPtr<ContainerNode>(LastBody());

      // 3. If the rows collection has zero elements in it, and this has no tbody elements in it:
      if (!parent)
      {
        // 3a. Let tbody be the result of creating a table element given this and "tbody".
        auto newBody = html::CreateTableElement<HTMLTableSectionElement>(document, u8"tbody");

        // 3b. Append tr to tbody.
        (void)newBody->AppendChild(*newRow);

        // 3c. Append tbody to this.
        if (auto result = AppendChild(*newBody); result.HasException())
        {
          return result.ReleaseException();
        }
        return newRow;
      }
    }
    if (auto result = parent->InsertBefore(*newRow, row.get()); result.HasException())
    {
      return result.ReleaseException();
    }

    // 7. Return tr.
    return newRow;
  }

  ExceptionOr<void> HTMLTableElement::DeleteRow(int32 index) noexcept
  {
    html::CEReactionsScope ceReactions;

    // 1. If index is less than −1 or greater than or equal to the number of elements in the rows collection,
    //    then throw an "IndexSizeError" DOMException.
    // (Found while walking the rows.)
    RefPtr<HTMLTableRowElement> row;

    // 2. If index is −1, then remove the last element in the rows collection from its parent, or do nothing
    // if
    //    the rows collection is empty.
    if (index == -1)
    {
      row = ShareRefPtr(html::HTMLTableRowsCollection::LastRow(*this));
      if (!row)
      {
        return {};
      }
    }
    else
    {
      for (int32 i = 0; i <= index; ++i)
      {
        row = ShareRefPtr(html::HTMLTableRowsCollection::RowAfter(*this, row.get()));
        if (!row)
        {
          break;
        }
      }
      if (!row)
      {
        return Exception {ExceptionCode::IndexSizeError};
      }
    }

    // 3. Otherwise, remove the indexth element in the rows collection from its parent.
    return row->Remove();
  }

#pragma endregion

#pragma region HTMLTableElement Obsolete members

  DOMString HTMLTableElement::Align() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableElement::Align(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"align", krys::move(value));
  }

  DOMString HTMLTableElement::Border() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"border").value_or(u8"");
  }

  void HTMLTableElement::Border(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"border", krys::move(value));
  }

  DOMString HTMLTableElement::Frame() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"frame").value_or(u8"");
  }

  void HTMLTableElement::Frame(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"frame", krys::move(value));
  }

  DOMString HTMLTableElement::Rules() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"rules").value_or(u8"");
  }

  void HTMLTableElement::Rules(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"rules", krys::move(value));
  }

  DOMString HTMLTableElement::Summary() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"summary").value_or(u8"");
  }

  void HTMLTableElement::Summary(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"summary", krys::move(value));
  }

  DOMString HTMLTableElement::Width() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLTableElement::Width(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"width", krys::move(value));
  }

  DOMString HTMLTableElement::BgColor() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"bgcolor").value_or(u8"");
  }

  void HTMLTableElement::BgColor(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"bgcolor", krys::move(value));
  }

  DOMString HTMLTableElement::CellPadding() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"cellpadding").value_or(u8"");
  }

  void HTMLTableElement::CellPadding(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"cellpadding", krys::move(value));
  }

  DOMString HTMLTableElement::CellSpacing() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"cellspacing").value_or(u8"");
  }

  void HTMLTableElement::CellSpacing(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"cellspacing", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  namespace
  {
    KRYS_NODISCARD bool HasHTMLTagName(const Element &element, html::TagName tagName) noexcept
    {
      return Is<HTMLElement>(element) && element.TagQualifiedName().TagName() == tagName;
    }

    KRYS_NODISCARD bool IsTableCellAncestor(const Element &element) noexcept
    {
      return HasHTMLTagName(element, html::TagName::thead) || HasHTMLTagName(element, html::TagName::tbody)
             || HasHTMLTagName(element, html::TagName::tfoot) || HasHTMLTagName(element, html::TagName::tr)
             || HasHTMLTagName(element, html::TagName::th);
    }

    bool SetTableCellsChanged(Element &element) noexcept
    {
      bool cellChanged = false;

      if (HasHTMLTagName(element, html::TagName::td))
      {
        cellChanged = true;
      }
      else if (IsTableCellAncestor(element))
      {
        for (auto child = element.FirstElementChild(); child; child = child->NextElementSibling())
        {
          cellChanged |= SetTableCellsChanged(*child);
        }
      }

      if (cellChanged)
      {
        element.InvalidateStyleForSubtree();
      }

      return cellChanged;
    }

    KRYS_NODISCARD bool GetBordersFromFrameAttributeValue(DOMStringView value, bool &borderTop,
                                                          bool &borderRight, bool &borderBottom,
                                                          bool &borderLeft) noexcept
    {
      using namespace html::presentational_hints;
      borderTop = false;
      borderRight = false;
      borderBottom = false;
      borderLeft = false;

      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"above"))
      {
        borderTop = true;
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"below"))
      {
        borderBottom = true;
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"hsides"))
      {
        borderTop = borderBottom = true;
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"vsides"))
      {
        borderLeft = borderRight = true;
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"lhs"))
      {
        borderLeft = true;
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"rhs"))
      {
        borderRight = true;
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"box")
               || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"border"))
      {
        borderTop = borderBottom = borderLeft = borderRight = true;
      }
      else if (!infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"void"))
      {
        return false;
      }
      return true;
    }

    KRYS_NODISCARD Ref<css::MutableStyleProperties> CreateBorderStyle(css::ValueId value) noexcept
    {
      auto style = css::MutableStyleProperties::Create();
      style->SetProperty(css::PropertyId::BorderTopStyle, value);
      style->SetProperty(css::PropertyId::BorderBottomStyle, value);
      style->SetProperty(css::PropertyId::BorderLeftStyle, value);
      style->SetProperty(css::PropertyId::BorderRightStyle, value);
      return style;
    }

    KRYS_NODISCARD Ref<css::MutableStyleProperties> CreateGroupBorderStyle(bool rows) noexcept
    {
      auto style = css::MutableStyleProperties::Create();
      if (rows)
      {
        style->SetProperty(css::PropertyId::BorderTopWidth, css::ValueId::Thin);
        style->SetProperty(css::PropertyId::BorderBottomWidth, css::ValueId::Thin);
        style->SetProperty(css::PropertyId::BorderTopStyle, css::ValueId::Solid);
        style->SetProperty(css::PropertyId::BorderBottomStyle, css::ValueId::Solid);
      }
      else
      {
        style->SetProperty(css::PropertyId::BorderLeftWidth, css::ValueId::Thin);
        style->SetProperty(css::PropertyId::BorderRightWidth, css::ValueId::Thin);
        style->SetProperty(css::PropertyId::BorderLeftStyle, css::ValueId::Solid);
        style->SetProperty(css::PropertyId::BorderRightStyle, css::ValueId::Solid);
      }
      return style;
    }
  }

  bool HTMLTableElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                            const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null
        && (localName == u8"width" || localName == u8"height" || localName == u8"bgcolor"
            || localName == u8"background" || localName == u8"valign" || localName == u8"vspace"
            || localName == u8"hspace" || localName == u8"cellspacing" || localName == u8"border"
            || localName == u8"bordercolor" || localName == u8"frame" || localName == u8"rules"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLTableElement::CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
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
    if (localName == u8"width")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::Width, value, AllowZeroValue::No);
      return;
    }
    if (localName == u8"height")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::Height, value);
      return;
    }
    if (localName == u8"border")
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::BorderWidth,
                                           ParseBorderWidthAttribute(value), css::UnitType::px);
      return;
    }
    if (localName == u8"bordercolor")
    {
      if (!value.empty())
      {
        AddHTMLColorToStyle(style, css::PropertyId::BorderColor, value);
      }
      return;
    }
    if (localName == u8"bgcolor")
    {
      AddHTMLColorToStyle(style, css::PropertyId::BackgroundColor, value);
      return;
    }
    if (localName == u8"background")
    {
      if (auto url = infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(value); !url.empty())
      {
        style.SetProperty(css::PropertyId::BackgroundImage,
                          css::ImageValue::Create(NodeDocument().CompleteUrl(url)));
      }
      return;
    }
    if (localName == u8"valign")
    {
      if (!value.empty())
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::VerticalAlign, ToCSSOMString(value));
      }
      return;
    }
    if (localName == u8"cellspacing")
    {
      if (!value.empty())
      {
        AddHTMLPixelsToStyle(style, css::PropertyId::BorderSpacing, value);
      }
      return;
    }
    if (localName == u8"align")
    {
      if (!value.empty())
      {
        if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"center"))
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::MarginInlineStart, css::ValueId::Auto);
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::MarginInlineEnd, css::ValueId::Auto);
        }
        else
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::Float, ToCSSOMString(value));
        }
      }
      return;
    }
    if (localName == u8"rules")
    {
      // The presence of a valid rules attribute causes border collapsing to be enabled.
      if (_rulesAttr != TableRules::Unset)
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::BorderCollapse, css::ValueId::Collapse);
      }
      return;
    }
    if (localName == u8"frame")
    {
      bool borderTop;
      bool borderRight;
      bool borderBottom;
      bool borderLeft;
      if (GetBordersFromFrameAttributeValue(value, borderTop, borderRight, borderBottom, borderLeft))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::BorderWidth, css::ValueId::Thin);
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::BorderTopStyle,
                                             borderTop ? css::ValueId::Solid : css::ValueId::Hidden);
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::BorderBottomStyle,
                                             borderBottom ? css::ValueId::Solid : css::ValueId::Hidden);
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::BorderLeftStyle,
                                             borderLeft ? css::ValueId::Solid : css::ValueId::Hidden);
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::BorderRightStyle,
                                             borderRight ? css::ValueId::Solid : css::ValueId::Hidden);
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

  void HTMLTableElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                            Maybe<DOMStringView> value,
                                            const DOMStringAtom &namespaceURI) noexcept
  {
    using namespace html::presentational_hints;
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != null)
    {
      return;
    }

    auto bordersBefore = GetCellBorders();
    auto oldPadding = _padding;
    auto newValue = value.value_or(DOMStringView {});

    if (localName == u8"border")
    {
      _borderAttr = ParseBorderWidthAttribute(value);
    }
    else if (localName == u8"frame")
    {
      bool borderTop;
      bool borderRight;
      bool borderBottom;
      bool borderLeft;
      _frameAttr =
        value
        && GetBordersFromFrameAttributeValue(newValue, borderTop, borderRight, borderBottom, borderLeft);
    }
    else if (localName == u8"rules")
    {
      _rulesAttr = TableRules::Unset;
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(newValue, u8"none"))
      {
        _rulesAttr = TableRules::None;
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(newValue, u8"groups"))
      {
        _rulesAttr = TableRules::Groups;
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(newValue, u8"rows"))
      {
        _rulesAttr = TableRules::Rows;
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(newValue, u8"cols"))
      {
        _rulesAttr = TableRules::Cols;
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(newValue, u8"all"))
      {
        _rulesAttr = TableRules::All;
      }
    }
    else if (localName == u8"cellpadding")
    {
      if (!newValue.empty())
      {
        auto padding = html::Numbers::ParseInteger(newValue);
        _padding = static_cast<uint16>(
          std::clamp<int64>(padding.Success() ? padding.Value : 0, 0, std::numeric_limits<uint16>::max()));
      }
      else
      {
        _padding = 1;
      }
    }

    if (bordersBefore != GetCellBorders() || oldPadding != _padding)
    {
      _sharedCellStyle = nullptr;
      bool cellChanged = false;
      for (auto child = FirstElementChild(); child; child = child->NextElementSibling())
      {
        cellChanged |= SetTableCellsChanged(*child);
      }
      if (cellChanged)
      {
        InvalidateStyleForSubtree();
      }
    }
  }

  const css::MutableStyleProperties *HTMLTableElement::AdditionalPresentationalHintStyle() const noexcept
  {
    if (_frameAttr)
    {
      return nullptr;
    }

    if (!_borderAttr)
    {
      // Setting the border to 'hidden' allows it to win over any border
      // set on the table's cells during border-conflict resolution.
      if (_rulesAttr != TableRules::Unset)
      {
        static NeverDestroyed<Ref<css::MutableStyleProperties>> solidBorderStyle =
          CreateBorderStyle(css::ValueId::Hidden);
        return solidBorderStyle->get();
      }
      return nullptr;
    }

    static NeverDestroyed<Ref<css::MutableStyleProperties>> outsetBorderStyle =
      CreateBorderStyle(css::ValueId::Outset);
    return outsetBorderStyle->get();
  }

  auto HTMLTableElement::GetCellBorders() const noexcept -> CellBorders
  {
    switch (_rulesAttr)
    {
      case TableRules::None:
      case TableRules::Groups:
      {
        return CellBorders::No;
      }
      case TableRules::All:
      {
        return CellBorders::Solid;
      }
      case TableRules::Cols:
      {
        return CellBorders::SolidColsOnly;
      }
      case TableRules::Rows:
      {
        return CellBorders::SolidRowsOnly;
      }
      case TableRules::Unset:
      {
        if (!_borderAttr)
        {
          return CellBorders::No;
        }
        return CellBorders::Inset;
      }
    }
    krys_unreachable();
  }

  Ref<css::MutableStyleProperties> HTMLTableElement::CreateSharedCellStyle() const noexcept
  {
    auto style = css::MutableStyleProperties::Create();

    switch (GetCellBorders())
    {
      case CellBorders::SolidColsOnly:
      {
        style->SetProperty(css::PropertyId::BorderLeftWidth, css::ValueId::Thin);
        style->SetProperty(css::PropertyId::BorderRightWidth, css::ValueId::Thin);
        style->SetProperty(css::PropertyId::BorderLeftStyle, css::ValueId::Solid);
        style->SetProperty(css::PropertyId::BorderRightStyle, css::ValueId::Solid);
        style->SetProperty(css::PropertyId::BorderColor, css::PrimitiveValue::Create(css::ValueId::Inherit));
        break;
      }
      case CellBorders::SolidRowsOnly:
      {
        style->SetProperty(css::PropertyId::BorderTopWidth, css::ValueId::Thin);
        style->SetProperty(css::PropertyId::BorderBottomWidth, css::ValueId::Thin);
        style->SetProperty(css::PropertyId::BorderTopStyle, css::ValueId::Solid);
        style->SetProperty(css::PropertyId::BorderBottomStyle, css::ValueId::Solid);
        style->SetProperty(css::PropertyId::BorderColor, css::PrimitiveValue::Create(css::ValueId::Inherit));
        break;
      }
      case CellBorders::Solid:
      {
        style->SetProperty(css::PropertyId::BorderWidth, css::PrimitiveValue::Create(1, css::UnitType::px));
        style->SetProperty(css::PropertyId::BorderStyle, css::PrimitiveValue::Create(css::ValueId::Solid));
        style->SetProperty(css::PropertyId::BorderColor, css::PrimitiveValue::Create(css::ValueId::Inherit));
        break;
      }
      case CellBorders::Inset:
      {
        style->SetProperty(css::PropertyId::BorderWidth, css::PrimitiveValue::Create(1, css::UnitType::px));
        style->SetProperty(css::PropertyId::BorderStyle, css::PrimitiveValue::Create(css::ValueId::Inset));
        style->SetProperty(css::PropertyId::BorderColor, css::PrimitiveValue::Create(css::ValueId::Inherit));
        break;
      }
      case CellBorders::No:
      {
        // If 'rules=none' then allow any borders set at cell level to take effect.
        break;
      }
    }

    if (_padding)
    {
      style->SetProperty(css::PropertyId::Padding, css::PrimitiveValue::Create(_padding, css::UnitType::px));
    }

    return style;
  }

  const css::MutableStyleProperties *HTMLTableElement::AdditionalCellStyle() const noexcept
  {
    if (!_sharedCellStyle)
    {
      _sharedCellStyle = CreateSharedCellStyle();
    }
    return _sharedCellStyle.get();
  }

  const css::MutableStyleProperties *HTMLTableElement::AdditionalGroupStyle(bool rows) const noexcept
  {
    if (_rulesAttr != TableRules::Groups)
    {
      return nullptr;
    }
    if (rows)
    {
      static NeverDestroyed<Ref<css::MutableStyleProperties>> rowBorderStyle = CreateGroupBorderStyle(true);
      return rowBorderStyle->get();
    }
    static NeverDestroyed<Ref<css::MutableStyleProperties>> columnBorderStyle = CreateGroupBorderStyle(false);
    return columnBorderStyle->get();
  }

#pragma endregion
}
