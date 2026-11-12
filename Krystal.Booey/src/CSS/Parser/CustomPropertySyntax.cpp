/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Parser/CustomPropertySyntax.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

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

    struct SyntaxTypeName
    {
      CSSOMStringView Name;
      CustomPropertySyntax::Type Type;
    };

    constexpr SyntaxTypeName SyntaxTypeNames[] = {
      {u8"angle", CustomPropertySyntax::Type::Angle},
      {u8"color", CustomPropertySyntax::Type::Color},
      {u8"custom-ident", CustomPropertySyntax::Type::CustomIdent},
      {u8"image", CustomPropertySyntax::Type::Image},
      {u8"integer", CustomPropertySyntax::Type::Integer},
      {u8"length", CustomPropertySyntax::Type::Length},
      {u8"length-percentage", CustomPropertySyntax::Type::LengthPercentage},
      {u8"number", CustomPropertySyntax::Type::Number},
      {u8"percentage", CustomPropertySyntax::Type::Percentage},
      {u8"resolution", CustomPropertySyntax::Type::Resolution},
      {u8"string", CustomPropertySyntax::Type::String},
      {u8"time", CustomPropertySyntax::Type::Time},
      {u8"transform-function", CustomPropertySyntax::Type::TransformFunction},
      {u8"transform-list", CustomPropertySyntax::Type::TransformList},
      {u8"url", CustomPropertySyntax::Type::URL},
    };

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
    for (auto &mapping : SyntaxTypeNames)
    {
      if (mapping.Name == dataTypeName)
      {
        return mapping.Type;
      }
    }
    return Type::Unknown;
  }

  CSSOMString CustomPropertySyntax::SyntaxString() const noexcept
  {
    if (IsUniversal())
    {
      return u8"*";
    }

    StringBuilder builder;
    CSSOMStringView separator = u8"";
    for (auto &component : Definition)
    {
      builder.Append(separator);
      separator = u8" | ";

      if (component.Ident != CSSOMStringAtom::Null())
      {
        SerialiseIdentifier(CSSOMString(component.Ident.View()), builder);
      }
      else
      {
        // Unknown types aren't kept by name, and fail to parse wherever a syntax is serialised.
        krys_debug_assert(component.Type != Type::Unknown);
        for (auto &mapping : SyntaxTypeNames)
        {
          if (mapping.Type == component.Type)
          {
            builder.Append(u8'<', mapping.Name, u8'>');
            break;
          }
        }
      }

      if (component.Multiplier == Multiplier::SpaceList)
      {
        builder.Append(u8'+');
      }
      else if (component.Multiplier == Multiplier::CommaList)
      {
        builder.Append(u8'#');
      }
    }
    return krys::move(builder).ToString();
  }

  CSSOMString CustomPropertySyntax::CSSTypeString() const noexcept
  {
    // 1. If the <css-type> consists of a single <syntax-component>, return the corresponding syntax string.
    if (Definition.size() == 1uz)
    {
      return SyntaxString();
    }

    // 2. Otherwise, return the concatenation of "type(", the corresponding syntax string and ")".
    StringBuilder builder;
    builder.Append(u8"type(", SyntaxString(), u8')');
    return krys::move(builder).ToString();
  }
}
