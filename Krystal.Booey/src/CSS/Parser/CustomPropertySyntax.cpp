#include "Krystal.Booey/CSS/Parser/CustomPropertySyntax.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"

namespace krys::boo::css
{
  namespace
  {
    void SkipUntil(CSSOMStringView &buffer, char8 character) noexcept
    {
      while (!buffer.empty() && buffer.front() != character)
      {
        buffer.remove_prefix(1uz);
      }
    }

    void SkipCSSSpaces(CSSOMStringView &buffer) noexcept
    {
      while (!buffer.empty() && IsCSSSpace(buffer.front()))
      {
        buffer.remove_prefix(1uz);
      }
    }

    /// @brief The part of `begin` that has been consumed to reach `buffer`.
    KRYS_NODISCARD CSSOMStringView ConsumedSince(CSSOMStringView begin, CSSOMStringView buffer) noexcept
    {
      return begin.substr(0uz, begin.size() - buffer.size());
    }
  }

  auto CustomPropertySyntax::ParseComponent(CSSOMStringView component) noexcept -> Maybe<Component>
  {
    auto buffer = component;

    auto consumeMultiplier = [&]
    {
      if (SkipExactly(buffer, u8'+'))
      {
        return Multiplier::SpaceList;
      }
      if (SkipExactly(buffer, u8'#'))
      {
        return Multiplier::CommaList;
      }
      return Multiplier::Single;
    };

    if (SkipExactly(buffer, u8'<'))
    {
      auto begin = buffer;
      SkipUntil(buffer, u8'>');
      if (buffer.size() == begin.size())
      {
        return {};
      }

      auto dataTypeName = ConsumedSince(begin, buffer);
      if (!SkipExactly(buffer, u8'>'))
      {
        return {};
      }

      auto multiplier = consumeMultiplier();

      SkipCSSSpaces(buffer);
      if (!buffer.empty())
      {
        return {};
      }

      auto type = TypeForTypeName(dataTypeName);

      // <transform-list> is a pre-multiplied data type.
      // https://drafts.css-houdini.org/css-properties-values-api/#multipliers
      if (type == Type::TransformList && multiplier != Multiplier::Single)
      {
        type = Type::Unknown;
      }

      return Component {.Type = type, .Multiplier = multiplier};
    }

    auto begin = buffer;
    while (!buffer.empty() && buffer.front() != u8'+' && buffer.front() != u8'#')
    {
      buffer.remove_prefix(1uz);
    }

    auto ident = [&]
    {
      InputStream input {CSSOMString(ConsumedSince(begin, buffer))};
      Tokenizer tokenizer {input};
      tokenizer.PumpTokenizer();

      auto range = tokenizer.Tokens();
      range.DiscardWhitespace();
      if (range.Peek().Type() != TokenType::Ident || !IsValidCustomIdentifier(range.Peek().ValueId()))
      {
        return CSSOMStringAtom::Null();
      }

      auto value = range.ConsumeIncludingWhitespace().IdentCodePoints();
      return range.IsAtEnd() ? CSSOMStringAtom {value} : CSSOMStringAtom::Null();
    }();

    if (ident == CSSOMStringAtom::Null())
    {
      return {};
    }

    auto multiplier = consumeMultiplier();

    return Component {.Type = Type::CustomIdent, .Multiplier = multiplier, .Ident = ident};
  }

  Maybe<CustomPropertySyntax> CustomPropertySyntax::Parse(CSSOMStringView syntax) noexcept
  {
    // The format doesn't quite parse with the tokenizer.
    auto buffer = syntax;
    SkipCSSSpaces(buffer);

    if (SkipExactly(buffer, u8'*'))
    {
      SkipCSSSpaces(buffer);
      if (!buffer.empty())
      {
        return {};
      }

      return Universal();
    }

    DefinitionType definition;

    while (!buffer.empty())
    {
      auto begin = buffer;

      SkipUntil(buffer, u8'|');

      auto component = ParseComponent(ConsumedSince(begin, buffer));
      if (!component)
      {
        return {};
      }

      definition.push_back(*component);

      (void)SkipExactly(buffer, u8'|');
      SkipCSSSpaces(buffer);
    }

    if (definition.empty())
    {
      return {};
    }

    return CustomPropertySyntax {krys::move(definition)};
  }

  Maybe<CustomPropertySyntax> CustomPropertySyntax::ConsumeType(TokenRange &tokens) noexcept
  {
    // https://drafts.csswg.org/css-mixins/#typedef-css-type
    // <css-type> = <syntax-component> | <type()>

    if (tokens.Peek().Type() == TokenType::Function)
    {
      // <type()> = type( <syntax> )
      if (tokens.Peek().FunctionId() != ValueId::Type)
      {
        return {};
      }

      auto typeRange = PropertyParserHelpers::ConsumeFunction(tokens);
      auto type = Parse(typeRange.Serialise(TokenSerialisationMode::Normal));
      if (!type || type->ContainsUnknownType())
      {
        return {};
      }
      return type;
    }

    auto typeRangeStart = tokens;
    while (!tokens.IsAtEnd())
    {
      auto peekType = tokens.Peek().Type();
      if (peekType == TokenType::Colon || peekType == TokenType::Comma || peekType == TokenType::Whitespace)
      {
        break;
      }
      (void)tokens.Consume();
    }
    tokens.DiscardWhitespace();

    auto typeRange = typeRangeStart.RangeUntil(tokens);
    auto type = Parse(typeRange.Serialise(TokenSerialisationMode::Normal));
    if (!type || type->Definition.size() != 1uz || type->ContainsUnknownType())
    {
      return {};
    }

    return type;
  }

  bool CustomPropertySyntax::ContainsUnknownType() const noexcept
  {
    for (auto &component : Definition)
    {
      if (component.Type == Type::Unknown)
      {
        return true;
      }
    }
    return false;
  }

  auto CustomPropertySyntax::TypeForTypeName(CSSOMStringView dataTypeName) noexcept -> Type
  {
    struct Mapping
    {
      CSSOMStringView Name;
      boo::css::CustomPropertySyntax::Type Type;
    };

    constexpr static Mapping mappings[] = {
      {u8"angle", Type::Angle},
      {u8"color", Type::Color},
      {u8"custom-ident", Type::CustomIdent},
      {u8"image", Type::Image},
      {u8"integer", Type::Integer},
      {u8"length", Type::Length},
      {u8"length-percentage", Type::LengthPercentage},
      {u8"number", Type::Number},
      {u8"percentage", Type::Percentage},
      {u8"resolution", Type::Resolution},
      {u8"string", Type::String},
      {u8"time", Type::Time},
      {u8"transform-function", Type::TransformFunction},
      {u8"transform-list", Type::TransformList},
      {u8"url", Type::URL},
    };

    for (auto &mapping : mappings)
    {
      if (mapping.Name == dataTypeName)
      {
        return mapping.Type;
      }
    }
    return Type::Unknown;
  }
}
