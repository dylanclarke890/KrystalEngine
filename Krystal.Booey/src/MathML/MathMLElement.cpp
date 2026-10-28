#include "Krystal.Booey/MathML/MathMLElement.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/MathML/MathMLNames.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>
#include <charconv>

namespace krys::boo::mathml
{
  namespace
  {
    // The largest value a non-negative integer reflects as.
    // https://html.spec.whatwg.org/#reflecting-content-attributes-in-idl-attributes:idl-unsigned-long
    constexpr uint32 MaxHTMLNonNegativeInteger = 2'147'483'647;
    // The largest rowspan a table cell takes, as HTML's.
    constexpr uint32 MaxMathMLRowspan = 65'534;

    KRYS_NODISCARD uint32 LimitToOnlyHTMLNonNegative(dom::DOMStringView value, uint32 defaultValue) noexcept
    {
      auto result = html::Numbers::ParseNonNegativeInteger(value);
      if (!result.Success() || result.Value > MaxHTMLNonNegativeInteger)
      {
        return defaultValue;
      }
      return static_cast<uint32>(result.Value);
    }
  }

  MathMLElement::MathMLElement(dom::Document &document, MathMLElementInterface interface,
                               dom::NodeFlags nodeFlags) noexcept
      : StyledElement(document, dom::NullQualifiedName(), nodeFlags | dom::NodeFlags::IsMathMLElement),
        _interface(interface)
  {
  }

  uint32 MathMLElement::ColSpan() const noexcept
  {
    if (!HasTagName(html::TagName::mtd))
    {
      return 1u;
    }
    auto colSpanValue = AttributeValue(MathMLNames::ColumnspanAttr()).value_or(dom::DOMString {});
    return std::max(1u, LimitToOnlyHTMLNonNegative(colSpanValue, 1u));
  }

  uint32 MathMLElement::RowSpan() const noexcept
  {
    if (!HasTagName(html::TagName::mtd))
    {
      return 1u;
    }
    auto rowSpanValue = AttributeValue(MathMLNames::RowspanAttr()).value_or(dom::DOMString {});
    return std::max(1u, std::min(LimitToOnlyHTMLNonNegative(rowSpanValue, 1u), MaxMathMLRowspan));
  }

  Maybe<dom::DOMString> MathMLElement::AttributeValue(const dom::QualifiedName &name) const noexcept
  {
    return GetAttributeNS(name.NamespaceURI(), name.LocalName());
  }

  void MathMLElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                         Maybe<dom::DOMStringView> value,
                                         dom::DOMStringAtom namespaceURI) noexcept
  {
    AttributeChanged(dom::QualifiedName {namespaceURI, dom::DOMStringAtom::Null(), localName}, oldValue,
                     value);
  }

  void MathMLElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                       Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == MathMLNames::ColumnspanAttr() || name == MathMLNames::RowspanAttr())
    {
      if (auto *cell = DynamicDowncast<RenderTableCell>(Renderer()); cell && HasTagName(html::TagName::mtd))
      {
        cell->ColSpanOrRowSpanChanged();
      }
    }

    StyledElement::OnAttributeChanged(name.LocalName(), oldValue, value, name.NamespaceURI());
  }

#pragma region Presentational hints

  namespace
  {
    KRYS_NODISCARD bool EndsWithIgnoringASCIICase(dom::DOMStringView value,
                                                  dom::DOMStringView suffix) noexcept
    {
      return value.size() >= suffix.size()
             && infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value.substr(value.size() - suffix.size()),
                                                                   suffix);
    }

    KRYS_NODISCARD bool IsDisallowedMathSizeAttribute(dom::DOMStringView value) noexcept
    {
      // FIXME: The CSS parser sometimes accept non-zero <number> font-size values on MathML elements, so
      // explicitly disallow them.
      auto number = value;
      while (!number.empty() && text::IsUnicodeCompatibleASCIIWhitespace(number.front()))
      {
        number.remove_prefix(1);
      }
      double unused {};
      const auto *first = reinterpret_cast<const char *>(number.data());
      const auto *last = first + number.size();
      auto result = std::from_chars(first, last, unused);
      if (!number.empty() && result.ec == std::errc {} && result.ptr == last && value != u8"0")
      {
        return true;
      }

      // Keywords from CSS font-size are disallowed.
      return infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"medium")
             || EndsWithIgnoringASCIICase(value, u8"large") || EndsWithIgnoringASCIICase(value, u8"small")
             || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"smaller")
             || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"larger")
             || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"math");
    }

    enum class ScriptLevelSyntax : uint8
    {
      Unsigned,
      AddUnsigned,
      AddMinusUnsigned,
    };
  }

  bool MathMLElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                         dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null()
        && (localName == MathMLNames::DirAttr().LocalName()
            || localName == MathMLNames::MathbackgroundAttr().LocalName()
            || localName == MathMLNames::MathcolorAttr().LocalName()
            || localName == MathMLNames::MathsizeAttr().LocalName()
            || localName == MathMLNames::DisplaystyleAttr().LocalName()
            || localName == MathMLNames::ScriptlevelAttr().LocalName()))
    {
      return true;
    }
    return StyledElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void MathMLElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                             dom::DOMStringAtom namespaceURI,
                                                             dom::DOMStringView value,
                                                             css::MutableStyleProperties &style) noexcept
  {
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      StyledElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }

    auto string = css::CSSOMString(value);
    if (localName == MathMLNames::MathbackgroundAttr().LocalName())
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::BackgroundColor, string);
      return;
    }
    if (localName == MathMLNames::MathsizeAttr().LocalName())
    {
      if (!IsDisallowedMathSizeAttribute(value))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::FontSize, string);
      }
      return;
    }
    if (localName == MathMLNames::MathcolorAttr().LocalName())
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::Color, string);
      return;
    }
    if (localName == MathMLNames::DirAttr().LocalName())
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::Direction, string);
      return;
    }
    if (localName == MathMLNames::DisplaystyleAttr().LocalName())
    {
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"false"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::MathStyle, css::ValueId::Compact);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"true"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::MathStyle, css::ValueId::Normal);
      }
      return;
    }
    // https://w3c.github.io/mathml-core/#dfn-scriptlevel
    if (localName == MathMLNames::ScriptlevelAttr().LocalName())
    {
      auto view = value;

      ScriptLevelSyntax syntax;
      if (value.starts_with(u8'+'))
      {
        syntax = ScriptLevelSyntax::AddUnsigned;
      }
      else if (value.starts_with(u8'-'))
      {
        syntax = ScriptLevelSyntax::AddMinusUnsigned;
      }
      else
      {
        syntax = ScriptLevelSyntax::Unsigned;
      }

      if (syntax == ScriptLevelSyntax::AddUnsigned || syntax == ScriptLevelSyntax::AddMinusUnsigned)
      {
        view = view.substr(1);
      }

      // Validate that the value (excluding an optional '+' or '-' initial character) is an integer
      if (view.empty() || !std::ranges::all_of(view, [](char8 c) { return text::IsASCIIDigit(c); }))
      {
        return;
      }

      switch (syntax)
      {
        case ScriptLevelSyntax::AddUnsigned:
        case ScriptLevelSyntax::AddMinusUnsigned:
        {
          StringBuilder builder;
          builder.Append(u8"add(");
          if (syntax == ScriptLevelSyntax::AddMinusUnsigned)
          {
            builder.Append(u8'-');
          }
          builder.Append(view);
          builder.Append(u8')');
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::MathDepth, builder.ToString());
          break;
        }
        case ScriptLevelSyntax::Unsigned:
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::MathDepth, string);
          break;
        }
      }
      return;
    }

    StyledElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion

  bool MathMLElement::ChildShouldCreateRenderer(const dom::Node &child) const noexcept
  {
    // In general, only MathML children are allowed. Text nodes are only visible in token MathML elements.
    return Is<MathMLElement>(child);
  }
}
