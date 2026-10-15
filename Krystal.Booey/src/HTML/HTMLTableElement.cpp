#include "Krystal.Booey/HTML/HTMLTableElement.hpp"
#include "algorithm"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::html
{
  HTMLTableElement::HTMLTableElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Table)
  {
  }

  HTMLTableElement::~HTMLTableElement() noexcept = default;

#pragma region HTMLTableElement

#pragma endregion

#pragma region HTMLTableElement Obsolete members

  dom::DOMString HTMLTableElement::Align() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableElement::Align(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  dom::DOMString HTMLTableElement::Border() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"border").value_or(u8"");
  }

  void HTMLTableElement::Border(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"border", krys::move(value));
  }

  dom::DOMString HTMLTableElement::Frame() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"frame").value_or(u8"");
  }

  void HTMLTableElement::Frame(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"frame", krys::move(value));
  }

  dom::DOMString HTMLTableElement::Rules() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"rules").value_or(u8"");
  }

  void HTMLTableElement::Rules(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"rules", krys::move(value));
  }

  dom::DOMString HTMLTableElement::Summary() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"summary").value_or(u8"");
  }

  void HTMLTableElement::Summary(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"summary", krys::move(value));
  }

  dom::DOMString HTMLTableElement::Width() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLTableElement::Width(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"width", krys::move(value));
  }

  dom::DOMString HTMLTableElement::BgColor() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"bgcolor").value_or(u8"");
  }

  void HTMLTableElement::BgColor(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"bgcolor", krys::move(value));
  }

  dom::DOMString HTMLTableElement::CellPadding() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"cellpadding").value_or(u8"");
  }

  void HTMLTableElement::CellPadding(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"cellpadding", krys::move(value));
  }

  dom::DOMString HTMLTableElement::CellSpacing() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"cellspacing").value_or(u8"");
  }

  void HTMLTableElement::CellSpacing(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"cellspacing", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  namespace
  {
    KRYS_NODISCARD bool GetBordersFromFrameAttributeValue(dom::DOMStringView value, bool &borderTop,
                                                          bool &borderRight, bool &borderBottom,
                                                          bool &borderLeft) noexcept
    {
      using namespace presentational_hints;
      borderTop = false;
      borderRight = false;
      borderBottom = false;
      borderLeft = false;

      if (EqualLettersIgnoringASCIICase(value, u8"above"))
      {
        borderTop = true;
      }
      else if (EqualLettersIgnoringASCIICase(value, u8"below"))
      {
        borderBottom = true;
      }
      else if (EqualLettersIgnoringASCIICase(value, u8"hsides"))
      {
        borderTop = borderBottom = true;
      }
      else if (EqualLettersIgnoringASCIICase(value, u8"vsides"))
      {
        borderLeft = borderRight = true;
      }
      else if (EqualLettersIgnoringASCIICase(value, u8"lhs"))
      {
        borderLeft = true;
      }
      else if (EqualLettersIgnoringASCIICase(value, u8"rhs"))
      {
        borderRight = true;
      }
      else if (EqualLettersIgnoringASCIICase(value, u8"box")
               || EqualLettersIgnoringASCIICase(value, u8"border"))
      {
        borderTop = borderBottom = borderLeft = borderRight = true;
      }
      else if (!EqualLettersIgnoringASCIICase(value, u8"void"))
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

  bool HTMLTableElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                            dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null()
        && (localName == u8"width" || localName == u8"height" || localName == u8"bgcolor"
            || localName == u8"background" || localName == u8"valign" || localName == u8"vspace"
            || localName == u8"hspace" || localName == u8"cellspacing" || localName == u8"border"
            || localName == u8"bordercolor" || localName == u8"frame" || localName == u8"rules"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLTableElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
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
                          css::ImageValue::Create(NodeDocument().CompleteURL(url)));
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
        if (EqualLettersIgnoringASCIICase(value, u8"center"))
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

  void HTMLTableElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                            Maybe<dom::DOMStringView> value,
                                            dom::DOMStringAtom namespaceURI) noexcept
  {
    using namespace presentational_hints;
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    auto bordersBefore = GetCellBorders();
    auto oldPadding = _padding;
    auto newValue = value.value_or(dom::DOMStringView {});

    if (localName == u8"border")
    {
      // FIXME: This attribute is a mess.
      _borderAttr = ParseBorderWidthAttribute(value);
    }
    else if (localName == u8"frame")
    {
      // FIXME: This attribute is a mess.
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
      if (EqualLettersIgnoringASCIICase(newValue, u8"none"))
      {
        _rulesAttr = TableRules::None;
      }
      else if (EqualLettersIgnoringASCIICase(newValue, u8"groups"))
      {
        _rulesAttr = TableRules::Groups;
      }
      else if (EqualLettersIgnoringASCIICase(newValue, u8"rows"))
      {
        _rulesAttr = TableRules::Rows;
      }
      else if (EqualLettersIgnoringASCIICase(newValue, u8"cols"))
      {
        _rulesAttr = TableRules::Cols;
      }
      else if (EqualLettersIgnoringASCIICase(newValue, u8"all"))
      {
        _rulesAttr = TableRules::All;
      }
    }
    else if (localName == u8"cellpadding")
    {
      if (!newValue.empty())
      {
        auto padding = Numbers::ParseInteger(newValue);
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
      // FIXME: Invalidate the style of the table's cells once there is style invalidation.
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
        return solidBorderStyle.get().get();
      }
      return nullptr;
    }

    static NeverDestroyed<Ref<css::MutableStyleProperties>> outsetBorderStyle =
      CreateBorderStyle(css::ValueId::Outset);
    return outsetBorderStyle.get().get();
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
      return rowBorderStyle.get().get();
    }
    static NeverDestroyed<Ref<css::MutableStyleProperties>> columnBorderStyle = CreateGroupBorderStyle(false);
    return columnBorderStyle.get().get();
  }

#pragma endregion
}
