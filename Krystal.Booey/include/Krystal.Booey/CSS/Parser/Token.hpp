#pragma once

#include "Krystal.Booey/CSS/Parser/NumericValue.hpp"
#include "Krystal.Booey/CSS/Parser/TokenSerialisationMode.hpp"
#include "Krystal.Booey/CSS/Parser/TokenType.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Text/Encodings/Encode.hpp"
#include "Krystal.Core/Text/Encodings/UTF.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::css
{
  class Token
  {
  private:
    TokenType _type : BitCount<TokenType>() {TokenType::Uninitialized};
    BlockTokenType _blockType : BitCount<BlockTokenType>() {BlockTokenType::None};
    HashTokenType _hashType : BitCount<HashTokenType>() {HashTokenType::Unrestricted};
    NumericValueType _numericValueType : BitCount<NumericValueType>() {NumericValueType::Integer};
    NumericSignChar _numericSignChar : BitCount<NumericSignChar>() {NumericSignChar::Missing};
    double _numericValue;
    SmallList<char8, 32uz> _codePoints;
    char32 _unicodeRangeStart;
    char32 _unicodeRangeEnd;

  public:
    Token(TokenType type) noexcept : _type(type)
    {
    }

    Token(TokenType type, BlockTokenType blockType) noexcept : _type(type), _blockType(blockType)
    {
    }

    Token(TokenType type, char32 codePoint) noexcept : _type(type)
    {
      assert(type == TokenType::Delim);

      AppendDataInternal(utf32_stringview(&codePoint, 1uz));
    }

    Token(TokenType type, utf32_stringview codePoints) noexcept : _type(type)
    {
      assert(_type == TokenType::Ident || _type == TokenType::Function || type == TokenType::Url
             || type == TokenType::String);

      AppendDataInternal(codePoints);
    }

    KRYS_NODISCARD TokenType Type() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD HashTokenType HashType() const noexcept
    {
      assert(_type == TokenType::Hash);
      return _hashType;
    }

    void HashType(HashTokenType hashType) noexcept
    {
      assert(_type == TokenType::Hash);
      _hashType = hashType;
    }

    KRYS_NODISCARD utf8_stringview IdentCodePoints() const noexcept
    {
      assert(_type == TokenType::Ident || _type == TokenType::Function || _type == TokenType::AtKeyword
             || _type == TokenType::Hash || _type == TokenType::String || _type == TokenType::Url
             || _type == TokenType::Delim);

      return utf8_stringview {_codePoints.data(), _codePoints.size()};
    }

    void IdentCodePoints(utf32_stringview codePoints) noexcept
    {
      assert(_type == TokenType::Ident || _type == TokenType::Function || _type == TokenType::AtKeyword
             || _type == TokenType::Hash || _type == TokenType::String || _type == TokenType::Url
             || _type == TokenType::Delim);

      AppendDataInternal(codePoints);
    }

    KRYS_NODISCARD css::ValueId ValueId() const noexcept
    {
      if (_type != TokenType::Ident)
      {
        return ValueId::Invalid;
      }

      return FindValueKeyword(IdentCodePoints());
    }

    KRYS_NODISCARD PropertyId PropertyId() const noexcept
    {
      if (_type != TokenType::Ident)
      {
        return PropertyId::Invalid;
      }

      return FindProperty(IdentCodePoints());
    }

    KRYS_NODISCARD css::ValueId FunctionId() const noexcept
    {
      if (_type != TokenType::Function)
      {
        return css::ValueId::Invalid;
      }

      return FindValueKeyword(IdentCodePoints());
    }

    void NumericValue(NumericValue value) noexcept
    {
      assert(_type == TokenType::Number || _type == TokenType::Percentage || _type == TokenType::Dimension);

      _numericValue = value.Value;
      _numericValueType = value.Type;
      _numericSignChar = value.SignCharacter;
    }

    KRYS_NODISCARD double NumericValue() const noexcept
    {
      assert(_type == TokenType::Number || _type == TokenType::Percentage || _type == TokenType::Dimension);

      return _numericValue;
    }

    KRYS_NODISCARD NumericValueType NumericValueType() const noexcept
    {
      assert(_type == TokenType::Number || _type == TokenType::Percentage || _type == TokenType::Dimension);

      return _numericValueType;
    }

    void Unit(utf32_stringview unit) noexcept
    {
      assert(_type == TokenType::Dimension);
      AppendDataInternal(unit);
    }

    KRYS_NODISCARD utf8_stringview Unit() const noexcept
    {
      assert(_type == TokenType::Dimension);
      return utf8_stringview {_codePoints.data(), _codePoints.size()};
    }

    void UnicodeRange(char32 start, char32 end) noexcept
    {
      assert(_type == TokenType::UnicodeRange);

      _unicodeRangeStart = start;
      _unicodeRangeEnd = end;
    }

    KRYS_NODISCARD bool IsBlockStart() const noexcept
    {
      return _blockType == BlockTokenType::Start;
    }

    KRYS_NODISCARD bool IsBlockEnd() const noexcept
    {
      return _blockType == BlockTokenType::End;
    }

    void Serialise(CSSOMString &builder, const Token *nextToken = nullptr,
                   TokenSerialisationMode mode = TokenSerialisationMode::Normal) const noexcept
    {
      //struct NextTokenNeedsCommentBuilder
      //{
      //  constexpr NextTokenNeedsCommentBuilder(std::initializer_list<TokenType> tokens) noexcept
      //  {
      //    for (auto token : tokens)
      //    {
      //      Buffer[static_cast<size_t>(token)] = true;
      //    }
      //  }

      //  Array<bool, EnumTraits<TokenType>::DistinctValues> Buffer {false};
      //};

      //// This is currently only used for @supports CSSOM. To keep our implementation
      //// simple we handle some of the edge cases incorrectly (see comments below).
      //auto AppendCommentIfNeeded = [&](const NextTokenNeedsCommentBuilder &tokensNeedingComment,
      //                                 auto... delimitersNeedingComment) noexcept
      //{
      //  if (!nextToken)
      //  {
      //    return;
      //  }

      //  TokenType nextType = nextToken->Type();
      //  if (tokensNeedingComment.Buffer[nextType])
      //  {
      //    builder.push_back(u8"/**/");
      //    return;
      //  }

      //  if (nextType == TokenType::Delim
      //      && ((delimitersNeedingComment == nextToken->IdentCodePoints()[0]) || ... || false))
      //  {
      //    builder.push_back(u8"/**/");
      //    return;
      //  }
      //};

      //switch (Type())
      //{
      //  case TokenType::Ident:
      //  {
      //    SerializeIdentifier(value().toString(), builder);
      //    AppendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl,
      //                           TokenType::Number, TokenType::Percentage, TokenType::Dimension,
      //                           TokenType::CDC, TokenType::OpenParen},
      //                          '-');
      //  }
      //    break;
      //  case TokenType::Function:
      //    SerializeIdentifier(value().toString(), builder);
      //    builder.append('(');
      //    break;
      //  case TokenType::AtKeyword:
      //    builder.append('@');
      //    SerializeIdentifier(value().toString(), builder);
      //    AppendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl, TokenType::Number,
      //                           TokenType::Percentage, TokenType::Dimension, TokenType::CDC},
      //                          '-');
      //    break;
      //  case TokenType::Hash:
      //    builder.append('#');
      //    SerializeIdentifier(value().toString(), builder, (getHashTokenType() == TokenType::HashUnrestricted));
      //    AppendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl, TokenType::Number,
      //                           TokenType::Percentage, TokenType::Dimension, TokenType::CDC},
      //                          '-');
      //    break;
      //  case TokenType::Url:
      //    builder.append("url("_s);
      //    SerializeIdentifier(value().toString(), builder);
      //    builder.append(')');
      //    break;
      //  case TokenType::Delimiter:
      //    switch (delimiter())
      //    {
      //      case '\\': builder.append("\\\n"_s); break;

      //      case '#':
      //      case '-':
      //        builder.append(delimiter());
      //        appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl, TokenType::Number,
      //                               TokenType::Percentage, TokenType::Dimension},
      //                              '-');
      //        break;

      //      case '@':
      //        builder.append('@');
      //        appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl}, '-');
      //        break;

      //      case '.':
      //      case '+':
      //        builder.append(delimiter());
      //        appendCommentIfNeeded({TokenType::Number, TokenType::Percentage, TokenType::Dimension});
      //        break;

      //      case '/':
      //        builder.append('/');
      //        // Weirdly Clang errors if you try to use the fold expression in
      //        // buildNextTokenNeedsCommentTable() because the true value is unused. So we just build the
      //        // table by hand here instead. See: rdar://69710661
      //        appendCommentIfNeeded({}, '*');
      //        break;

      //      default: builder.append(delimiter()); break;
      //    }
      //    break;
      //  case TokenType::Number:
      //    if (mode == SerializationMode::CustomProperty)
      //      builder.append(originalText());
      //    else
      //    {
      //      if (m_numericSign == PlusSign)
      //        builder.append('+');
      //      builder.append(numericValue());
      //    }
      //    appendCommentIfNeeded(
      //      {TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl, TokenType::Number, TokenType::Percentage, TokenType::Dimension},
      //      '%');
      //    break;
      //  case TokenType::Percentage:
      //    if (mode == SerializationMode::CustomProperty)
      //      builder.append(originalText(), '%');
      //    else
      //      builder.append(numericValue(), '%');
      //    break;
      //  case TokenType::Dimension:
      //    if (mode == SerializationMode::CustomProperty && m_nonUnitPrefixLength)
      //      builder.append(originalText());
      //    else
      //    {
      //      builder.append(numericValue());
      //      serializeIdentifier(unitString().toString(), builder);
      //    }
      //    appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl, TokenType::Number,
      //                           TokenType::Percentage, TokenType::Dimension, TokenType::CDC},
      //                          '-');
      //    break;
      //  case TokenType::String:         serializeString(value().toString(), builder); break;

      //  case TokenType::IncludeMatch:   builder.append("~="_s); break;
      //  case TokenType::DashMatch:      builder.append("|="_s); break;
      //  case TokenType::PrefixMatch:    builder.append("^="_s); break;
      //  case TokenType::SuffixMatch:    builder.append("$="_s); break;
      //  case TokenType::SubstringMatch: builder.append("*="_s); break;
      //  case TokenType::Column:         builder.append("||"_s); break;
      //  case TokenType::CDO:            builder.append("<!--"_s); break;
      //  case TokenType::CDC:            builder.append("-->"_s); break;
      //  case TokenType::BadString:      builder.append("'\n"_s); break;
      //  case TokenType::BadUrl:         builder.append("url(()"_s); break;
      //  case TokenType::NonNewlineWhitespace:
      //  {
      //    auto count = mode == SerializationMode::CustomProperty ? m_whitespaceCount : 1;
      //    for (decltype(count) i = 0; i < count; ++i)
      //      builder.append(' ');
      //    break;
      //  }
      //  case TokenType::Newline:          builder.append(mode == SerializationMode::CustomProperty ? '\n' : ' '); break;
      //  case TokenType::Colon:            builder.append(':'); break;
      //  case TokenType::Semicolon:        builder.append(';'); break;
      //  case TokenType::Comma:            builder.append(','); break;
      //  case TokenType::LeftParenthesis:  builder.append('('); break;
      //  case TokenType::RightParenthesis: builder.append(')'); break;
      //  case TokenType::LeftBracket:      builder.append('['); break;
      //  case TokenType::RightBracket:     builder.append(']'); break;
      //  case TokenType::LeftBrace:        builder.append('{'); break;
      //  case TokenType::RightBrace:       builder.append('}'); break;

      //  case TokenType::EndOfFile:
      //  case TokenType::Comment:          ASSERT_NOT_REACHED(); break;
      //}
    }

  private:
    void AppendDataInternal(utf32_stringview codePoints)
    {
      auto encodeResult = krys::text::Encode<krys::text::UTF8>(codePoints);
      krys_debug_assert(encodeResult.Error != krys::text::EncodeError::None);
      _codePoints.assign(encodeResult.Output.begin(), encodeResult.Output.end());
    }
  };
}