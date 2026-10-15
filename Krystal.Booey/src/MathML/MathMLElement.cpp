#include "Krystal.Booey/MathML/MathMLElement.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>
#include <charconv>
#include <format>

namespace krys::boo::mathml
{
  MathMLElement::MathMLElement(dom::Document &document) noexcept
      : StyledElement(document, dom::NullQualifiedName(), dom::NodeFlags::IsMathMLElement)
  {
  }

#pragma region Presentational hints

  namespace
  {
    KRYS_NODISCARD bool IsOneOf(dom::DOMStringView value,
                                std::initializer_list<dom::DOMStringView> values) noexcept
    {
      return std::ranges::find(values, value) != values.end();
    }

    KRYS_NODISCARD css::CSSOMString ConvertMathSizeIfNeeded(dom::DOMStringView value) noexcept
    {
      if (value == u8"small")
      {
        return u8"0.75em";
      }
      if (value == u8"normal")
      {
        return u8"1em";
      }
      if (value == u8"big")
      {
        return u8"1.5em";
      }

      // FIXME: mathsize accepts any MathML length, including named spaces (see parseMathMLLength).
      // FIXME: Might be better to use double than float.
      // FIXME: Might be better to use "shortest" numeric formatting instead of fixed width.
      float unitlessValue {};
      const auto *first = reinterpret_cast<const char *>(value.data());
      const auto *last = first + value.size();
      auto result = std::from_chars(first, last, unitlessValue);
      if (result.ec != std::errc {} || result.ptr != last)
      {
        return css::CSSOMString(value);
      }
      auto percentage = std::format("{:.3f}%", unitlessValue * 100);
      return css::CSSOMString(reinterpret_cast<const char8 *>(percentage.data()), percentage.size());
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
        && IsOneOf(localName.View(), {u8"background", u8"color", u8"dir", u8"fontfamily", u8"fontsize",
                                      u8"fontstyle", u8"fontweight", u8"mathbackground", u8"mathcolor",
                                      u8"mathsize", u8"displaystyle", u8"scriptlevel"}))
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
    if (localName == u8"mathbackground")
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::BackgroundColor, string);
      return;
    }
    if (localName == u8"mathsize")
    {
      // The coreMathMLEnabled setting is off by default.
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::FontSize, ConvertMathSizeIfNeeded(value));
      return;
    }
    if (localName == u8"mathcolor")
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::Color, string);
      return;
    }
    if (localName == u8"dir")
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::Direction, string);
      return;
    }
    if (localName == u8"displaystyle")
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
    if (localName == u8"scriptlevel")
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
          auto builder = css::CSSOMString(u8"add(");
          if (syntax == ScriptLevelSyntax::AddMinusUnsigned)
          {
            builder += u8'-';
          }
          builder += view;
          builder += u8')';
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::MathDepth, builder);
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

    // FIXME: The following are deprecated attributes that should lose if there is a conflict with a
    // non-deprecated attribute.
    if (localName == u8"fontsize")
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::FontSize, string);
    }
    else if (localName == u8"background")
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::BackgroundColor, string);
    }
    else if (localName == u8"color")
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::Color, string);
    }
    else if (localName == u8"fontstyle")
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::FontStyle, string);
    }
    else if (localName == u8"fontweight")
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::FontWeight, string);
    }
    else if (localName == u8"fontfamily")
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::FontFamily, string);
    }
    else
    {
      StyledElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
    }
  }

#pragma endregion
}
