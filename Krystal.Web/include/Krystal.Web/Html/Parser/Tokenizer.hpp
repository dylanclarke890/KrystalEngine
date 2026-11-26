/*
 * Copyright (C) 2008, 2015 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Google, Inc. All rights reserved.
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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Containers/SmallList.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Utils/IsEOF.hpp"
#include "Krystal.Web/Html/Parser/CharacterReference.hpp"
#include "Krystal.Web/Html/Parser/InputStream.hpp"
#include "Krystal.Web/Html/Parser/Token.hpp"
#include <algorithm>
#include <ranges>

namespace krys::web::html
{
  enum class ParseError : uint8
  {
    UnexpectedNullCharacter,
    EOFBeforeTagName,
    InvalidFirstCharacterOfTagName,
    MissingEndTagName,
    EOFInTag,
    EOFInScriptHTMLCommentLikeText,
    UnexpectedEqualsSignBeforeAttributeName,
    UnexpectedCharacterInAttributeName,
    DuplicateAttribute,
    MissingAttributeValue,
    UnexpectedCharacterInUnquotedAttributeValue,
    MissingWhitespaceBetweenAttributes,
    UnexpectedSolidusInTag,
    CDATAInHTMLContent,
    IncorrectlyOpenedComment,
    AbruptClosingOfEmptyComment,
    EOFInComment,
    NestedComment,
    IncorrectlyClosedComment,
    EOFInDOCTYPE,
    MissingWhitespaceBeforeDOCTYPEName,
    InvalidCharacterSequenceAfterDOCTYPEName,
    MissingDOCTYPEName,
    MissingWhitespaceAfterDOCTYPEPublicKeyword,
    MissingDOCTYPEPublicIdentifier,
    MissingQuoteBeforeDOCTYPEPublicIdentifier,
    AbruptDOCTYPEPublicIdentifier,
    MissingWhitespaceBetweenDOCTYPEPublicAndSystemIdentifiers,
    MissingQuoteBeforeDOCTYPESystemIdentifier,
    MissingWhitespaceAfterDOCTYPESystemKeyword,
    MissingDOCTYPESystemIdentifier,
    AbruptDOCTYPESystemIdentifier,
    UnexpectedCharacterAfterDOCTYPESystemIdentifier,
    EOFInCDATA,
    EOFInProcessingInstruction,
    InvalidFirstCharacterOfProcessingInstructionTarget,
    InvalidProcessingInstructionTarget,
    DisallowedProcessingInstructionTarget,
    UnknownNamedCharacterReference,
    AbsenceOfDigitsInNumericCharacterReference,
    MissingSemicolonAfterCharacterReference,
    NullCharacterReference,
    CharacterReferenceOutsideUnicodeRange,
    SurrogateCharacterReference,
    NonCharacterCharacterReference,
    ControlCharacterReference,
  };

  consteval ParseError MaxEnumValue(ParseError) noexcept
  {
    return ParseError::ControlCharacterReference;
  }

  enum class TokenizerState : uint8
  {
    Data,
    RCDATA,
    RAWTEXT,
    ScriptData,
    PLAINTEXT,
    TagOpen,
    EndTagOpen,
    TagName,
    RCDATALessThanSign,
    RCDATAEndTagOpen,
    RCDATAEndTagName,
    RAWTEXTLessThanSign,
    RAWTEXTEndTagOpen,
    RAWTEXTEndTagName,
    ScriptDataLessThanSign,
    ScriptDataEndTagOpen,
    ScriptDataEndTagName,
    ScriptDataEscapeStart,
    ScriptDataEscapeStartDash,
    ScriptDataEscaped,
    ScriptDataEscapedDash,
    ScriptDataEscapedDashDash,
    ScriptDataEscapedLessThanSign,
    ScriptDataEscapedEndTagOpen,
    ScriptDataEscapedEndTagName,
    ScriptDataDoubleEscapeStart,
    ScriptDataDoubleEscaped,
    ScriptDataDoubleEscapedDash,
    ScriptDataDoubleEscapedDashDash,
    ScriptDataDoubleEscapedLessThanSign,
    ScriptDataDoubleEscapeEnd,
    BeforeAttributeName,
    AttributeName,
    AfterAttributeName,
    BeforeAttributeValue,
    AttributeValueDoubleQuoted,
    AttributeValueSingleQuoted,
    AttributeValueUnquoted,
    AfterAttributeValueQuoted,
    SelfClosingStartTag,
    BogusComment,
    MarkupDeclarationOpen,
    CommentStart,
    CommentStartDash,
    Comment,
    CommentLessThanSign,
    CommentLessThanSignBang,
    CommentLessThanSignBangDash,
    CommentLessThanSignBangDashDash,
    CommentEndDash,
    CommentEnd,
    CommentEndBang,
    DOCTYPE,
    BeforeDOCTYPEName,
    DOCTYPEName,
    AfterDOCTYPEName,
    AfterDOCTYPEPublicKeyword,
    BeforeDOCTYPEPublicIdentifier,
    DOCTYPEPublicIdentifierDoubleQuoted,
    DOCTYPEPublicIdentifierSingleQuoted,
    AfterDOCTYPEPublicIdentifier,
    BetweenDOCTYPEPublicAndSystemIdentifiers,
    AfterDOCTYPESystemKeyword,
    BeforeDOCTYPESystemIdentifier,
    DOCTYPESystemIdentifierDoubleQuoted,
    DOCTYPESystemIdentifierSingleQuoted,
    AfterDOCTYPESystemIdentifier,
    BogusDOCTYPE,
    CDATASection,
    CDATASectionBracket,
    CDATASectionEnd,
    ProcessingInstructionOpen,
    ProcessingInstructionTarget,
    AfterProcessingInstructionTarget,
    ProcessingInstructionData,
    ProcessingInstructionQuestionable,
    CharacterReference,
    NamedCharacterReference,
    AmbiguousAmpersand,
    NumericCharacterReference,
    HexadecimalCharacterReferenceStart,
    DecimalCharacterReferenceStart,
    HexadecimalCharacterReference,
    DecimalCharacterReference,
    NumericCharacterReferenceEnd,
  };

  consteval TokenizerState MaxEnumValue(TokenizerState) noexcept
  {
    return TokenizerState::NumericCharacterReferenceEnd;
  }

  class NextTokenPtr
  {
    friend class Tokenizer;

  private:
    Token *_token {nullptr};

    explicit NextTokenPtr(Token *token) noexcept : _token(token)
    {
    }

  public:
    NextTokenPtr() noexcept = default;

    NextTokenPtr(NextTokenPtr &&other) noexcept : _token(std::exchange(other._token, nullptr))
    {
    }

    ~NextTokenPtr() noexcept
    {
      if (_token != nullptr)
      {
        _token->Clear();
      }
    }

    NextTokenPtr &operator=(NextTokenPtr &&) = delete;

    void Clear() noexcept
    {
      if (_token != nullptr)
      {
        _token->Clear();
        _token = nullptr;
      }
    }

    operator bool() const noexcept
    {
      return _token != nullptr;
    }

    KRYS_NODISCARD Token &operator*() const noexcept
    {
      krys_debug_assert(_token != nullptr);
      return *_token;
    }

    KRYS_NODISCARD Token *operator->() const noexcept
    {
      krys_debug_assert(_token != nullptr);
      return _token;
    }
  };

  struct TokenizerError
  {
    ParseError Error;
    SourceLocation Location {};
  };

  class Tokenizer
  {
  private:
    TokenizerState _state : EnumBitWidth<TokenizerState> {TokenizerState::Data};
    TokenizerState _characterReferenceReturnState : EnumBitWidth<TokenizerState> {TokenizerState::Data};
    Token _token;
    InputStream &_input;
    SmallList<char32, 32u> _appropriateEndTagName;
    SmallList<char32, 32u> _bufferedEndTagName;
    Func<bool()> _isCDATASectionAllowed {nullptr};

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#character-reference-code
    int64 _characterReferenceCode {0};

    /// @brief The largest character reference code kept: one past the Unicode range.
    constexpr static int64 MaxCharacterReferenceCode = 0x110000;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#temporary-buffer
    SmallList<char32, 32u> _temporaryBuffer;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#named-character-reference-state
    Span<const NamedCharacterReferenceEntry> _namedCharacterReferenceMatchEntries;
    const NamedCharacterReferenceEntry *_longestCharacterReferenceMatch {nullptr};

    List<TokenizerError> _parseErrors;

  public:
    Tokenizer(InputStream &input, Func<bool()> &&isCDATASectionAllowed) noexcept
        : _input(input), _isCDATASectionAllowed(krys::move(isCDATASectionAllowed))
    {
    }

    NextTokenPtr NextToken() noexcept
    {
      return NextTokenPtr(ProcessToken() ? &_token : nullptr);
    }

    KRYS_NODISCARD TokenizerState State() const noexcept
    {
      return _state;
    }

    void State(TokenizerState state) noexcept
    {
      _state = state;
    }

    KRYS_NODISCARD const List<TokenizerError> &ParseErrors() const noexcept
    {
      return _parseErrors;
    }

  private:
    KRYS_NODISCARD bool ProcessToken() noexcept
    {
      if (!_bufferedEndTagName.empty() && !IsInEndTagBufferingState())
      {
        // We are back here after emitting a character token that came just before an end tag.
        // To continue parsing the end tag we need to move the buffered tag name into the token.
        FlushBufferedEndTag();

        // If we are in the data state, the end tag is already complete and we should emit it
        // now, otherwise, we want to resume parsing the partial end tag.
        if (_state == TokenizerState::Data)
        {
          return true;
        }
      }

      if (!_input.Peek())
      {
        return HasBufferedCharacterToken();
      }

      return StepTokenizationStateMachine();
    }

    KRYS_NODISCARD bool HasBufferedCharacterToken() const noexcept
    {
      return _token.Type() == TokenType::Character;
    }

    bool IsInEndTagBufferingState() const
    {
      switch (_state)
      {
        case TokenizerState::RCDATAEndTagOpen:
        case TokenizerState::RCDATAEndTagName:
        case TokenizerState::RAWTEXTEndTagOpen:
        case TokenizerState::RAWTEXTEndTagName:
        case TokenizerState::ScriptDataEndTagOpen:
        case TokenizerState::ScriptDataEndTagName:
        case TokenizerState::ScriptDataEscapedEndTagOpen:
        case TokenizerState::ScriptDataEscapedEndTagName: return true;
        default:                                          return false;
      }
    }

    void AppendToTemporaryBuffer(const auto &characters)
    {
      _temporaryBuffer.append_range(characters);
    }

    void BufferCharacter(char32 character) noexcept
    {
      krys_debug_assert(character != EOFMarker);
      _token.AppendToCharacters(character);
    }

    template <size_t N>
    void BufferCharacters(Array<char32, N> characters) noexcept
    {
#if KRYS_DEBUG
      for (auto character : characters)
      {
        krys_debug_assert(character != EOFMarker);
      }
#endif
      _token.AppendToCharacters(characters);
    }

    void BufferCharacters(Span<char32> characters) noexcept
    {
#if KRYS_DEBUG
      for (auto character : characters)
      {
        krys_debug_assert(character != EOFMarker);
      }
#endif
      _token.AppendToCharacters(characters);
    }

    void BeginAttribute()
    {
      _token.BeginAttribute();
    }

    void EndAttribute()
    {
      auto &attributes = _token.Attributes();
      auto it = std::ranges::find_if(attributes.begin(), attributes.end(), [&](const auto &attribute)
                                     { return attribute.Name == _token.CurrentAttribute()->Name; });
      if (std::distance(attributes.begin(), it) < static_cast<std::ptrdiff_t>(attributes.size() - 1uz))
      {
        // Duplicate attribute found; remove the last one.
        ParserError(ParseError::DuplicateAttribute);
        attributes.pop_back();
      }

      _token.EndAttribute();
    }

    void ParserError(ParseError error) noexcept
    {
      _parseErrors.push_back({
        .Error = error,
        .Location = _input.GetCurrentLocation(),
      });
    }

    KRYS_NODISCARD static bool IsTokenizerWhitespace(char32 character) noexcept
    {
      return character == ' ' || character == '\x0A' || character == '\x09' || character == '\x0C';
    }

    KRYS_NODISCARD bool IsAppropriateEndTag() const noexcept
    {
      return _bufferedEndTagName == _appropriateEndTagName;
    }

    void AppendToPossibleEndTag(char32 character) noexcept
    {
      _bufferedEndTagName.push_back(character);
    }

    void FlushBufferedEndTag() noexcept
    {
      _token.BeginEndTag(_bufferedEndTagName);
      _bufferedEndTagName.clear();
      _appropriateEndTagName.clear();
      _temporaryBuffer.clear();
    }

    KRYS_NODISCARD bool CommitToPartialEndTag(char32 character, TokenizerState state) noexcept
    {
      krys_debug_assert(_input.NextInputCharacter() == character);
      _temporaryBuffer.push_back(character);
      _input.Advance();

      if (HasBufferedCharacterToken())
      {
        // Emit the buffered token so that on the next pass we can continue parsing the end tag.
        _state = state;
        return true;
      }

      FlushBufferedEndTag();
      return false;
    }

    KRYS_NODISCARD bool CommitToCompleteEndTag() noexcept
    {
      krys_debug_assert(_input.NextInputCharacter() == U'>');
      _temporaryBuffer.push_back(U'>');

      _input.Advance();
      _state = TokenizerState::Data;

      if (HasBufferedCharacterToken())
      {
        // Emit the character token so that on the next pass we can emit the end tag.
        return true;
      }

      FlushBufferedEndTag();
      return true;
    }

    KRYS_NODISCARD bool IsTemporaryBufferEqualTo(const utf32_string &str) const noexcept
    {
      return std::ranges::equal(_temporaryBuffer, str);
    }

    KRYS_NODISCARD bool CharacterReferenceWasConsumedAsPartOfAnAttribute() const noexcept
    {
      return _characterReferenceReturnState == TokenizerState::AttributeValueDoubleQuoted
             || _characterReferenceReturnState == TokenizerState::AttributeValueSingleQuoted
             || _characterReferenceReturnState == TokenizerState::AttributeValueUnquoted;
    }

    void FlushCodePointsConsumedAsACharacterReference() noexcept
    {
      if (CharacterReferenceWasConsumedAsPartOfAnAttribute())
      {
        _token.AppendToCurrentAttributeValue(_temporaryBuffer);
      }
      else
      {
        _token.AppendToCharacters(_temporaryBuffer);
      }
      _temporaryBuffer.clear();
    }

    KRYS_NODISCARD bool EmitTagToken() noexcept
    {
      krys_debug_assert(_token.Type() == TokenType::StartTag || _token.Type() == TokenType::EndTag);
      if (_token.Type() == TokenType::StartTag)
      {
        _appropriateEndTagName.clear();
        for (auto ch : _token.Name())
        {
          _appropriateEndTagName.push_back(ch);
        }
      }

      _state = TokenizerState::Data;
      _input.Advance();

      return true;
    }

    KRYS_NODISCARD bool EmitDOCTYPEToken(bool consumeCurrentCharacter) noexcept
    {
      krys_debug_assert(_token.Type() == TokenType::DOCTYPE);
      _state = TokenizerState::Data;

      if (!consumeCurrentCharacter)
      {
        return true;
      }

      _input.Advance();
      return true;
    }

    KRYS_NODISCARD bool EmitCommentToken(bool consumeCurrentCharacter) noexcept
    {
      krys_debug_assert(_token.Type() == TokenType::Comment);
      _state = TokenizerState::Data;

      if (!consumeCurrentCharacter)
      {
        return true;
      }

      _input.Advance();
      return true;
    }

    KRYS_NODISCARD bool EmitProcessingInstructionToken() noexcept
    {
      krys_debug_assert(_token.Type() == TokenType::ProcessingInstruction);
      _state = TokenizerState::Data;
      _input.Advance();
      return true;
    }

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#convert-the-temporary-buffer-to-a-comment
    void ConvertTemporaryBufferToComment() noexcept
    {
      _token.BeginComment();
      _token.AppendToComment(U'?');
      if (!_temporaryBuffer.empty())
      {
        _token.AppendToComment(_temporaryBuffer);
      }
    }

    KRYS_NODISCARD bool IsTemporaryBufferASCIICaseInsensitiveMatch(utf32_stringview value) const noexcept
    {
      return std::ranges::equal(_temporaryBuffer, value,
                                [](char32 a, char32 b) { return krys::text::ToASCIILower(a) == b; });
    }

    KRYS_NODISCARD bool EmitEOFToken() noexcept
    {
      _state = TokenizerState::Data;

      _token.Clear();
      _token.SetAsEOF();

      _input.Advance();

      return true;
    }

#pragma region State Machine Macros

#define KRYS_BEGIN_STATE(stateName)                                                                          \
  case TokenizerState::stateName:                                                                            \
  stateName:                                                                                                 \
  {                                                                                                          \
    constexpr auto currentState = TokenizerState::stateName;                                                 \
    (void)currentState;

#define KRYS_END_STATE()                                                                                     \
  krys_unreachable();                                                                                        \
  break;                                                                                                     \
  }

#define KRYS_RETURN_IN_CURRENT_STATE(expression)                                                             \
  do                                                                                                         \
  {                                                                                                          \
    _state = currentState;                                                                                   \
    return expression;                                                                                       \
  } while (false)

#define KRYS_RETURN_IN_CURRENT_STATE_IF_CHARACTERS_BUFFERED()                                                \
  do                                                                                                         \
  {                                                                                                          \
    if (HasBufferedCharacterToken())                                                                         \
    {                                                                                                        \
      _state = currentState;                                                                                 \
      return true;                                                                                           \
    }                                                                                                        \
  } while (false)

// For when the HTML spec says "reconsume the current input character in the <mumble> state."
#define KRYS_RECONSUME_IN(newState)                                                                          \
  do                                                                                                         \
  {                                                                                                          \
    goto newState;                                                                                           \
  } while (false)

// For when the HTML spec says "consume the next input character ... and switch to the <mumble> state."
#define KRYS_ADVANCE_TO(newState)                                                                            \
  do                                                                                                         \
  {                                                                                                          \
    if (!_input.Advance())                                                                                   \
    {                                                                                                        \
      _state = TokenizerState::newState;                                                                     \
      return HasBufferedCharacterToken();                                                                    \
    }                                                                                                        \
    character = _input.NextInputCharacter();                                                                 \
    goto newState;                                                                                           \
  } while (false)

// For when the HTML spec says "consume the next input character ... and switch to the <mumble> state."
#define KRYS_ADVANCE_PAST_NON_NEWLINE_TO(newState)                                                           \
  do                                                                                                         \
  {                                                                                                          \
    if (!_input.Advance())                                                                                   \
    {                                                                                                        \
      _state = TokenizerState::newState;                                                                     \
      return HasBufferedCharacterToken();                                                                    \
    }                                                                                                        \
    character = _input.NextInputCharacter();                                                                 \
    goto newState;                                                                                           \
  } while (false)

// For more complex cases, caller consumes the characters first and then uses this macro.
#define KRYS_SWITCH_TO(newState)                                                                             \
  do                                                                                                         \
  {                                                                                                          \
    if (!_input.Peek())                                                                                      \
    {                                                                                                        \
      _state = TokenizerState::newState;                                                                     \
      return HasBufferedCharacterToken();                                                                    \
    }                                                                                                        \
    character = _input.NextInputCharacter();                                                                 \
    goto newState;                                                                                           \
  } while (false)

#define KRYS_RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE()                                                 \
  do                                                                                                         \
  {                                                                                                          \
    if (_characterReferenceReturnState == TokenizerState::Data)                                              \
    {                                                                                                        \
      KRYS_RECONSUME_IN(Data);                                                                               \
    }                                                                                                        \
    if (_characterReferenceReturnState == TokenizerState::RCDATA)                                            \
    {                                                                                                        \
      KRYS_RECONSUME_IN(RCDATA);                                                                             \
    }                                                                                                        \
    if (_characterReferenceReturnState == TokenizerState::AttributeValueDoubleQuoted)                        \
    {                                                                                                        \
      KRYS_RECONSUME_IN(AttributeValueDoubleQuoted);                                                         \
    }                                                                                                        \
    if (_characterReferenceReturnState == TokenizerState::AttributeValueSingleQuoted)                        \
    {                                                                                                        \
      KRYS_RECONSUME_IN(AttributeValueSingleQuoted);                                                         \
    }                                                                                                        \
    if (_characterReferenceReturnState == TokenizerState::AttributeValueUnquoted)                            \
    {                                                                                                        \
      KRYS_RECONSUME_IN(AttributeValueUnquoted);                                                             \
    }                                                                                                        \
  } while (false)

#define KRYS_ADVANCE_TO_CHARACTER_REFERENCE_RETURN_STATE()                                                   \
  do                                                                                                         \
  {                                                                                                          \
    if (_characterReferenceReturnState == TokenizerState::Data)                                              \
    {                                                                                                        \
      KRYS_ADVANCE_TO(Data);                                                                                 \
    }                                                                                                        \
    if (_characterReferenceReturnState == TokenizerState::RCDATA)                                            \
    {                                                                                                        \
      KRYS_ADVANCE_TO(RCDATA);                                                                               \
    }                                                                                                        \
    if (_characterReferenceReturnState == TokenizerState::AttributeValueDoubleQuoted)                        \
    {                                                                                                        \
      KRYS_ADVANCE_TO(AttributeValueDoubleQuoted);                                                           \
    }                                                                                                        \
    if (_characterReferenceReturnState == TokenizerState::AttributeValueSingleQuoted)                        \
    {                                                                                                        \
      KRYS_ADVANCE_TO(AttributeValueSingleQuoted);                                                           \
    }                                                                                                        \
    if (_characterReferenceReturnState == TokenizerState::AttributeValueUnquoted)                            \
    {                                                                                                        \
      KRYS_ADVANCE_TO(AttributeValueUnquoted);                                                               \
    }                                                                                                        \
  } while (false)

#pragma endregion

#pragma warning(push)
#pragma warning(disable : 4'702) // Disable unreachable code warning

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#tokenization
    KRYS_NODISCARD bool StepTokenizationStateMachine() noexcept
    {
      constexpr char32 Null = U'\000';
      constexpr char32 Ampersand = U'&';
      constexpr char32 LessThanSign = U'<';
      constexpr char32 GreaterThanSign = U'>';
      constexpr char32 Solidus = U'/';
      constexpr char32 QuestionMark = U'?';
      constexpr char32 ExclamationMark = U'!';
      constexpr char32 HyphenMinus = U'-';
      constexpr char32 LowLine = U'_';
      constexpr char32 EqualSign = U'=';
      constexpr char32 QuotationMark = U'"';
      constexpr char32 Apostrophe = U'\'';
      constexpr char32 GraveAccent = U'`';
      constexpr char32 RightSquareBracket = U']';
      constexpr char32 NumberSign = U'#';
      constexpr char32 Semicolon = U';';
      constexpr char32 EndOfFile = EOFMarker;
      constexpr char32 Replacement = krys::text::UnicodeReplacementChar;

      char32 character = _input.NextInputCharacter();
      switch (_state)
      {
        KRYS_BEGIN_STATE(Data)
        if (character == Ampersand)
        {
          _characterReferenceReturnState = TokenizerState::Data;
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CharacterReference);
        }
        if (character == LessThanSign)
        {
          KRYS_RETURN_IN_CURRENT_STATE_IF_CHARACTERS_BUFFERED();
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(TagOpen);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          KRYS_RETURN_IN_CURRENT_STATE_IF_CHARACTERS_BUFFERED();
          return EmitEOFToken();
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          BufferCharacter(Null); // parser error but we still emit the null as per the spec.
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(Data);
        }

        BufferCharacter(character);
        KRYS_ADVANCE_TO(Data);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(RCDATA)
        if (character == Ampersand)
        {
          _characterReferenceReturnState = TokenizerState::RCDATA;
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CharacterReference);
        }
        if (character == LessThanSign)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(RCDATALessThanSign);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          KRYS_RECONSUME_IN(Data);
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          BufferCharacter(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(RCDATA);
        }

        BufferCharacter(character);
        KRYS_ADVANCE_TO(RCDATA);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(RAWTEXT)
        if (character == LessThanSign)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(RAWTEXTLessThanSign);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          KRYS_RECONSUME_IN(Data);
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          BufferCharacter(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(RAWTEXT);
        }

        BufferCharacter(character);
        KRYS_ADVANCE_TO(RAWTEXT);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptData)
        if (character == LessThanSign)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataLessThanSign);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          KRYS_RECONSUME_IN(Data);
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          BufferCharacter(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptData);
        }

        BufferCharacter(character);
        KRYS_ADVANCE_TO(ScriptData);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(PLAINTEXT)
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          KRYS_RECONSUME_IN(Data);
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          BufferCharacter(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(PLAINTEXT);
        }

        BufferCharacter(character);
        KRYS_ADVANCE_TO(PLAINTEXT);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(TagOpen)
        if (character == ExclamationMark)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(MarkupDeclarationOpen);
        }
        if (character == Solidus)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(EndTagOpen);
        }
        if (krys::text::IsASCIIAlpha(character))
        {
          _token.BeginStartTag(krys::text::ToASCIILower(character));
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(TagName);
        }
        if (character == QuestionMark)
        {
          _temporaryBuffer.clear();
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ProcessingInstructionOpen);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFBeforeTagName);
          BufferCharacter(LessThanSign);
          KRYS_RECONSUME_IN(Data);
        }

        ParserError(ParseError::InvalidFirstCharacterOfTagName);
        BufferCharacter(LessThanSign);
        KRYS_RECONSUME_IN(Data);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(EndTagOpen)
        if (krys::text::IsASCIIAlpha(character))
        {
          _token.BeginEndTag(krys::text::ToASCIILower(character));
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(TagName);
        }
        if (character == GreaterThanSign)
        {
          ParserError(ParseError::MissingEndTagName);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(Data); // We don't emit the greater than sign, as per the spec.
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFBeforeTagName);
          BufferCharacters(Array {LessThanSign, Solidus});
          KRYS_RECONSUME_IN(Data);
        }

        ParserError(ParseError::InvalidFirstCharacterOfTagName);
        _token.BeginComment();
        KRYS_RECONSUME_IN(BogusComment);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(TagName)
        if (IsTokenizerWhitespace(character))
        {
          KRYS_ADVANCE_TO(BeforeAttributeName);
        }
        if (character == Solidus)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(SelfClosingStartTag);
        }
        if (character == GreaterThanSign)
        {
          return EmitTagToken();
        }
        if (krys::text::IsASCIIUpper(character))
        {
          _token.AppendToName(krys::text::ToASCIILowerUnchecked(character));
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(TagName);
        }
        if (character == Null)
        {
          _token.AppendToName(Replacement);
          ParserError(ParseError::UnexpectedNullCharacter);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(TagName);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInTag);
          KRYS_RECONSUME_IN(Data);
        }

        _token.AppendToName(character);
        KRYS_ADVANCE_PAST_NON_NEWLINE_TO(TagName);

        KRYS_END_STATE()

        KRYS_BEGIN_STATE(RCDATALessThanSign)
        if (character == Solidus)
        {
          _temporaryBuffer.clear();
          krys_debug_assert(_bufferedEndTagName.empty());
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(RCDATAEndTagOpen);
        }

        BufferCharacter(LessThanSign);
        KRYS_RECONSUME_IN(RCDATA);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(RCDATAEndTagOpen)
        if (krys::text::IsASCIIAlpha(character))
        {
          _temporaryBuffer.push_back(character);
          AppendToPossibleEndTag(krys::text::ToASCIILower(character));
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(RCDATAEndTagName);
        }

        BufferCharacters(Array {LessThanSign, Solidus});
        KRYS_RECONSUME_IN(RCDATA);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(RCDATAEndTagName)
        if (IsTokenizerWhitespace(character))
        {
          if (IsAppropriateEndTag())
          {
            if (CommitToPartialEndTag(character, TokenizerState::BeforeAttributeName))
            {
              return true;
            }
            KRYS_SWITCH_TO(BeforeAttributeName);
          }
        }
        if (character == Solidus)
        {
          if (IsAppropriateEndTag())
          {
            if (CommitToPartialEndTag(Solidus, TokenizerState::SelfClosingStartTag))
            {
              return true;
            }
            KRYS_SWITCH_TO(SelfClosingStartTag);
          }
        }
        if (character == GreaterThanSign)
        {
          if (IsAppropriateEndTag())
          {
            return CommitToCompleteEndTag();
          }
        }
        if (krys::text::IsASCIIAlpha(character))
        {
          _temporaryBuffer.push_back(character);
          AppendToPossibleEndTag(krys::text::ToASCIILower(character));
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(RCDATAEndTagName);
        }

        BufferCharacters(Array {LessThanSign, Solidus});
        BufferCharacters(_temporaryBuffer);
        _bufferedEndTagName.clear();
        _temporaryBuffer.clear();
        KRYS_RECONSUME_IN(RCDATA);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(RAWTEXTLessThanSign)
        if (character == Solidus)
        {
          _temporaryBuffer.clear();
          krys_debug_assert(_bufferedEndTagName.empty());
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(RAWTEXTEndTagOpen);
        }

        BufferCharacter(LessThanSign);
        KRYS_RECONSUME_IN(RAWTEXT);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(RAWTEXTEndTagOpen)
        if (krys::text::IsASCIIAlpha(character))
        {
          _temporaryBuffer.push_back(character);
          AppendToPossibleEndTag(krys::text::ToASCIILower(character));
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(RAWTEXTEndTagName);
        }

        BufferCharacters(Array {LessThanSign, Solidus});
        KRYS_RECONSUME_IN(RAWTEXT);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(RAWTEXTEndTagName)
        if (IsTokenizerWhitespace(character))
        {
          if (IsAppropriateEndTag())
          {
            if (CommitToPartialEndTag(character, TokenizerState::BeforeAttributeName))
            {
              return true;
            }
            KRYS_SWITCH_TO(BeforeAttributeName);
          }
        }
        if (character == Solidus)
        {
          if (IsAppropriateEndTag())
          {
            if (CommitToPartialEndTag(Solidus, TokenizerState::SelfClosingStartTag))
            {
              return true;
            }
            KRYS_SWITCH_TO(SelfClosingStartTag);
          }
        }
        if (character == GreaterThanSign)
        {
          if (IsAppropriateEndTag())
          {
            return CommitToCompleteEndTag();
          }
        }
        if (krys::text::IsASCIIAlpha(character))
        {
          _temporaryBuffer.push_back(character);
          AppendToPossibleEndTag(krys::text::ToASCIILower(character));
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(RAWTEXTEndTagName);
        }

        BufferCharacters(Array {LessThanSign, Solidus});
        BufferCharacters(_temporaryBuffer);
        _bufferedEndTagName.clear();
        _temporaryBuffer.clear();
        KRYS_RECONSUME_IN(RAWTEXT);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptDataLessThanSign)
        if (character == Solidus)
        {
          _temporaryBuffer.clear();
          krys_debug_assert(_bufferedEndTagName.empty());
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEndTagOpen);
        }
        if (character == ExclamationMark)
        {
          BufferCharacters(Array {LessThanSign, ExclamationMark});
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapeStart);
        }

        BufferCharacter(LessThanSign);
        KRYS_RECONSUME_IN(ScriptData);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptDataEndTagOpen)
        if (krys::text::IsASCIIAlpha(character))
        {
          _temporaryBuffer.push_back(character);
          AppendToPossibleEndTag(krys::text::ToASCIILower(character));
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEndTagName);
        }

        BufferCharacters(Array {LessThanSign, Solidus});
        KRYS_RECONSUME_IN(ScriptData);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptDataEndTagName)
        if (IsTokenizerWhitespace(character))
        {
          if (IsAppropriateEndTag())
          {
            if (CommitToPartialEndTag(character, TokenizerState::BeforeAttributeName))
            {
              return true;
            }
            KRYS_SWITCH_TO(BeforeAttributeName);
          }
        }
        if (character == Solidus)
        {
          if (IsAppropriateEndTag())
          {
            if (CommitToPartialEndTag(Solidus, TokenizerState::SelfClosingStartTag))
            {
              return true;
            }
            KRYS_SWITCH_TO(SelfClosingStartTag);
          }
        }
        if (character == GreaterThanSign)
        {
          if (IsAppropriateEndTag())
          {
            return CommitToCompleteEndTag();
          }
        }
        if (krys::text::IsASCIIAlpha(character))
        {
          _temporaryBuffer.push_back(character);
          AppendToPossibleEndTag(krys::text::ToASCIILower(character));
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEndTagName);
        }

        BufferCharacters(Array {LessThanSign, Solidus});
        BufferCharacters(_temporaryBuffer);
        _bufferedEndTagName.clear();
        _temporaryBuffer.clear();
        KRYS_RECONSUME_IN(ScriptData);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptDataEscapeStart)
        if (character == HyphenMinus)
        {
          BufferCharacter(HyphenMinus);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapeStartDash);
        }

        KRYS_RECONSUME_IN(ScriptData);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptDataEscapeStartDash)
        if (character == HyphenMinus)
        {
          BufferCharacter(HyphenMinus);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedDashDash);
        }

        KRYS_RECONSUME_IN(ScriptData);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptDataEscaped)
        if (character == HyphenMinus)
        {
          BufferCharacter(HyphenMinus);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedDash);
        }
        if (character == LessThanSign)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedLessThanSign);
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          BufferCharacter(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscaped);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInScriptHTMLCommentLikeText);
          KRYS_RECONSUME_IN(Data);
        }

        BufferCharacter(character);
        KRYS_ADVANCE_TO(ScriptDataEscaped);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptDataEscapedDash)
        if (character == HyphenMinus)
        {
          BufferCharacter(HyphenMinus);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedDashDash);
        }
        if (character == LessThanSign)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedLessThanSign);
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          BufferCharacter(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscaped);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInScriptHTMLCommentLikeText);
          KRYS_RECONSUME_IN(Data);
        }

        BufferCharacter(character);
        KRYS_ADVANCE_TO(ScriptDataEscaped);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptDataEscapedDashDash)
        if (character == HyphenMinus)
        {
          BufferCharacter(HyphenMinus);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedDashDash);
        }
        if (character == LessThanSign)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedLessThanSign);
        }
        if (character == GreaterThanSign)
        {
          BufferCharacter(GreaterThanSign);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptData);
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          BufferCharacter(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscaped);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInScriptHTMLCommentLikeText);
          KRYS_RECONSUME_IN(Data);
        }

        BufferCharacter(character);
        KRYS_ADVANCE_TO(ScriptDataEscaped);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptDataEscapedLessThanSign)
        if (character == Solidus)
        {
          _temporaryBuffer.clear();
          krys_debug_assert(_bufferedEndTagName.empty());
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedEndTagOpen);
        }
        if (krys::text::IsASCIIAlpha(character))
        {
          _temporaryBuffer.clear();
          BufferCharacter(LessThanSign);
          KRYS_RECONSUME_IN(ScriptDataDoubleEscapeStart);
        }

        BufferCharacter(LessThanSign);
        KRYS_RECONSUME_IN(ScriptDataEscaped);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptDataEscapedEndTagOpen)
        if (krys::text::IsASCIIAlpha(character))
        {
          _temporaryBuffer.push_back(character);
          AppendToPossibleEndTag(krys::text::ToASCIILower(character));
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedEndTagName);
        }

        BufferCharacters(Array {LessThanSign, Solidus});
        KRYS_RECONSUME_IN(ScriptDataEscaped);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptDataEscapedEndTagName)
        if (IsTokenizerWhitespace(character))
        {
          if (IsAppropriateEndTag())
          {
            if (CommitToPartialEndTag(character, TokenizerState::BeforeAttributeName))
            {
              return true;
            }
            KRYS_SWITCH_TO(BeforeAttributeName);
          }
        }
        if (character == Solidus)
        {
          if (IsAppropriateEndTag())
          {
            if (CommitToPartialEndTag(Solidus, TokenizerState::SelfClosingStartTag))
            {
              return true;
            }
            KRYS_SWITCH_TO(SelfClosingStartTag);
          }
        }
        if (character == GreaterThanSign)
        {
          if (IsAppropriateEndTag())
          {
            return CommitToCompleteEndTag();
          }
        }
        if (krys::text::IsASCIIAlpha(character))
        {
          _temporaryBuffer.push_back(character);
          AppendToPossibleEndTag(krys::text::ToASCIILower(character));
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedEndTagName);
        }

        BufferCharacters(Array {LessThanSign, Solidus});
        BufferCharacters(_temporaryBuffer);
        _bufferedEndTagName.clear();
        _temporaryBuffer.clear();
        KRYS_RECONSUME_IN(ScriptDataEscaped);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptDataDoubleEscapeStart)
        if (IsTokenizerWhitespace(character) || character == Solidus || character == GreaterThanSign)
        {
          BufferCharacter(character);
          if (IsTemporaryBufferEqualTo(U"script"))
          {
            KRYS_ADVANCE_TO(ScriptDataDoubleEscaped);
          }
          else
          {
            KRYS_ADVANCE_TO(ScriptDataEscaped);
          }
        }
        if (krys::text::IsASCIIAlpha(character))
        {
          BufferCharacter(character);
          _temporaryBuffer.push_back(krys::text::ToASCIILower(character));
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapeStart);
        }

        KRYS_RECONSUME_IN(ScriptDataEscaped);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptDataDoubleEscaped)
        if (character == HyphenMinus)
        {
          BufferCharacter(HyphenMinus);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapedDash);
        }
        if (character == LessThanSign)
        {
          BufferCharacter(LessThanSign);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapedLessThanSign);
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          BufferCharacter(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscaped);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInScriptHTMLCommentLikeText);
          KRYS_RECONSUME_IN(Data);
        }

        BufferCharacter(character);
        KRYS_ADVANCE_TO(ScriptDataDoubleEscaped);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptDataDoubleEscapedDash)
        if (character == HyphenMinus)
        {
          BufferCharacter(HyphenMinus);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapedDashDash);
        }
        if (character == LessThanSign)
        {
          BufferCharacter(LessThanSign);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapedLessThanSign);
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          BufferCharacter(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscaped);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInScriptHTMLCommentLikeText);
          KRYS_RECONSUME_IN(Data);
        }

        BufferCharacter(character);
        KRYS_ADVANCE_TO(ScriptDataDoubleEscaped);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptDataDoubleEscapedDashDash)
        if (character == HyphenMinus)
        {
          BufferCharacter(HyphenMinus);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapedDashDash);
        }
        if (character == LessThanSign)
        {
          BufferCharacter(LessThanSign);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapedLessThanSign);
        }
        if (character == GreaterThanSign)
        {
          BufferCharacter(GreaterThanSign);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptData);
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          BufferCharacter(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscaped);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInScriptHTMLCommentLikeText);
          KRYS_RECONSUME_IN(Data);
        }

        BufferCharacter(character);
        KRYS_ADVANCE_TO(ScriptDataDoubleEscaped);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptDataDoubleEscapedLessThanSign)
        if (character == Solidus)
        {
          _temporaryBuffer.clear();
          krys_debug_assert(_bufferedEndTagName.empty());
          BufferCharacter(Solidus);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapeEnd);
        }

        KRYS_RECONSUME_IN(ScriptDataDoubleEscaped);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ScriptDataDoubleEscapeEnd)
        if (IsTokenizerWhitespace(character) || character == Solidus || character == GreaterThanSign)
        {
          BufferCharacter(character);
          if (IsTemporaryBufferEqualTo(U"script"))
          {
            KRYS_ADVANCE_TO(ScriptDataEscaped);
          }
          else
          {
            KRYS_ADVANCE_TO(ScriptDataDoubleEscaped);
          }
        }
        if (krys::text::IsASCIIAlpha(character))
        {
          BufferCharacter(character);
          _temporaryBuffer.push_back(krys::text::ToASCIILower(character));
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapeEnd);
        }

        KRYS_RECONSUME_IN(ScriptDataDoubleEscaped);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(BeforeAttributeName)
        if (IsTokenizerWhitespace(character))
        {
          KRYS_ADVANCE_TO(BeforeAttributeName);
        }
        if (character == Solidus || character == GreaterThanSign)
        {
          KRYS_RECONSUME_IN(AfterAttributeName);
        }
        if (character == EqualSign)
        {
          ParserError(ParseError::UnexpectedEqualsSignBeforeAttributeName);
          _token.BeginAttribute();
          _token.AppendToCurrentAttributeName(EqualSign);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(AttributeName);
        }

        _token.BeginAttribute();
        KRYS_RECONSUME_IN(AttributeName);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(AttributeName)
        if (IsTokenizerWhitespace(character) || character == Solidus || character == GreaterThanSign
            || character == EndOfFile)
        {
          KRYS_RECONSUME_IN(AfterAttributeName);
        }
        if (character == EqualSign)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(BeforeAttributeValue);
        }
        if (krys::text::IsASCIIUpper(character))
        {
          _token.AppendToCurrentAttributeName(krys::text::ToASCIILowerUnchecked(character));
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(AttributeName);
        }
        if (character == Null)
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          _token.AppendToCurrentAttributeName(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(AttributeName);
        }
        if (character == QuotationMark || character == Apostrophe || character == LessThanSign)
        {
          ParserError(ParseError::UnexpectedCharacterInAttributeName);
          // Treat it as per 'anything else' case below.
        }

        _token.AppendToCurrentAttributeName(character);
        KRYS_ADVANCE_PAST_NON_NEWLINE_TO(AttributeName);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(AfterAttributeName)
        if (IsTokenizerWhitespace(character))
        {
          // Ignore the character.
          KRYS_ADVANCE_TO(AfterAttributeName);
        }
        if (character == Solidus)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(SelfClosingStartTag);
        }
        if (character == EqualSign)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(BeforeAttributeValue);
        }
        if (character == GreaterThanSign)
        {
          return EmitTagToken();
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInTag);
          KRYS_RECONSUME_IN(Data);
        }

        EndAttribute();
        BeginAttribute();
        KRYS_RECONSUME_IN(AttributeName);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(BeforeAttributeValue)
        if (IsTokenizerWhitespace(character))
        {
          // Ignore the character.
          KRYS_ADVANCE_TO(BeforeAttributeValue);
        }
        if (character == QuotationMark)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(AttributeValueDoubleQuoted);
        }
        if (character == Apostrophe)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(AttributeValueSingleQuoted);
        }
        if (character == GreaterThanSign)
        {
          ParserError(ParseError::MissingAttributeValue);
          EndAttribute();
          return EmitTagToken();
        }

        KRYS_RECONSUME_IN(AttributeValueUnquoted);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(AttributeValueDoubleQuoted)
        if (character == QuotationMark)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(AfterAttributeValueQuoted);
        }
        if (character == Ampersand)
        {
          _characterReferenceReturnState = TokenizerState::AttributeValueDoubleQuoted;
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CharacterReference);
        }
        if (character == Null)
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          _token.AppendToCurrentAttributeValue(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(AttributeValueDoubleQuoted);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInTag);
          EndAttribute();
          KRYS_RECONSUME_IN(Data);
        }

        _token.AppendToCurrentAttributeValue(character);
        KRYS_ADVANCE_TO(AttributeValueDoubleQuoted);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(AttributeValueSingleQuoted)
        if (character == Apostrophe)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(AfterAttributeValueQuoted);
        }
        if (character == Ampersand)
        {
          _characterReferenceReturnState = TokenizerState::AttributeValueSingleQuoted;
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CharacterReference);
        }
        if (character == Null)
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          _token.AppendToCurrentAttributeValue(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(AttributeValueSingleQuoted);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInTag);
          EndAttribute();
          KRYS_RECONSUME_IN(Data);
        }

        _token.AppendToCurrentAttributeValue(character);
        KRYS_ADVANCE_TO(AttributeValueSingleQuoted);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(AttributeValueUnquoted)
        if (IsTokenizerWhitespace(character))
        {
          EndAttribute();
          KRYS_ADVANCE_TO(BeforeAttributeName);
        }
        if (character == Ampersand)
        {
          _characterReferenceReturnState = TokenizerState::AttributeValueUnquoted;
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CharacterReference);
        }
        if (character == GreaterThanSign)
        {
          EndAttribute();
          return EmitTagToken();
        }
        if (character == Null)
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          _token.AppendToCurrentAttributeValue(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(AttributeValueUnquoted);
        }
        if (character == QuotationMark || character == Apostrophe || character == LessThanSign
            || character == EqualSign || character == GraveAccent)
        {
          ParserError(ParseError::UnexpectedCharacterInUnquotedAttributeValue);
          _token.AppendToCurrentAttributeValue(character);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(AttributeValueUnquoted);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInTag);
          EndAttribute();
          KRYS_RECONSUME_IN(Data);
        }

        _token.AppendToCurrentAttributeValue(character);
        KRYS_ADVANCE_PAST_NON_NEWLINE_TO(AttributeValueUnquoted);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(AfterAttributeValueQuoted)
        if (IsTokenizerWhitespace(character))
        {
          EndAttribute();
          KRYS_ADVANCE_TO(BeforeAttributeName);
        }
        if (character == Solidus)
        {
          EndAttribute();
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(SelfClosingStartTag);
        }
        if (character == GreaterThanSign)
        {
          EndAttribute();
          return EmitTagToken();
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInTag);
          EndAttribute();
          KRYS_RECONSUME_IN(Data);
        }

        ParserError(ParseError::MissingWhitespaceBetweenAttributes);
        EndAttribute();
        KRYS_RECONSUME_IN(BeforeAttributeName);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(SelfClosingStartTag)
        if (character == GreaterThanSign)
        {
          // Set the self-closing flag of the current tag token. (An end tag's flag is only a parse error,
          // so it's left unset.)
          if (_token.Type() == TokenType::StartTag)
          {
            _token.SetSelfClosingFlag();
          }
          return EmitTagToken();
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInTag);
          KRYS_RECONSUME_IN(Data);
        }

        ParserError(ParseError::UnexpectedSolidusInTag);

        if (_token.CurrentAttribute() != nullptr)
        {
          EndAttribute();
        }
        KRYS_RECONSUME_IN(BeforeAttributeName);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(BogusComment)
        if (character == GreaterThanSign)
        {
          return EmitCommentToken(true);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          return EmitCommentToken(false);
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          _token.AppendToComment(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(BogusComment);
        }

        _token.AppendToComment(character);
        KRYS_ADVANCE_PAST_NON_NEWLINE_TO(BogusComment);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(MarkupDeclarationOpen)
        if (character == HyphenMinus)
        {
          auto result = _input.AdvancePast<false>("--");
          if (result == InputStream::MatchResult::Matched)
          {
            _token.BeginComment();
            KRYS_SWITCH_TO(CommentStart);
          }
          if (result == InputStream::MatchResult::NotEnoughCharacters)
          {
            KRYS_RETURN_IN_CURRENT_STATE(HasBufferedCharacterToken());
          }
        }
        else if (krys::text::IsASCIICaselessEqual(character, 'd'))
        {
          auto result = _input.AdvancePast("doctype");
          if (result == InputStream::MatchResult::Matched)
          {
            KRYS_SWITCH_TO(DOCTYPE);
          }
          if (result == InputStream::MatchResult::NotEnoughCharacters)
          {
            KRYS_RETURN_IN_CURRENT_STATE(HasBufferedCharacterToken());
          }
        }
        else if (character == '[')
        {
          auto result = _input.AdvancePast<false>("[CDATA[");
          if (result == InputStream::MatchResult::Matched)
          {
            if (_isCDATASectionAllowed())
            {
              KRYS_SWITCH_TO(CDATASection);
            }
            else
            {
              ParserError(ParseError::CDATAInHTMLContent);
              _token.BeginComment();
              _token.AppendToComment("[CDATA[");
              KRYS_SWITCH_TO(BogusComment);
            }
          }
          if (result == InputStream::MatchResult::NotEnoughCharacters)
          {
            KRYS_RETURN_IN_CURRENT_STATE(HasBufferedCharacterToken());
          }
        }

        ParserError(ParseError::IncorrectlyOpenedComment);
        _token.BeginComment();
        KRYS_RECONSUME_IN(BogusComment);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(CommentStart)
        if (character == HyphenMinus)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CommentStartDash);
        }
        if (character == GreaterThanSign)
        {
          ParserError(ParseError::AbruptClosingOfEmptyComment);
          return EmitCommentToken(true);
        }

        KRYS_RECONSUME_IN(Comment);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(CommentStartDash)
        if (character == HyphenMinus)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CommentEnd);
        }
        if (character == GreaterThanSign)
        {
          ParserError(ParseError::AbruptClosingOfEmptyComment);
          return EmitCommentToken(true);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInComment);
          return EmitCommentToken(false);
        }

        _token.AppendToComment(HyphenMinus);
        KRYS_RECONSUME_IN(Comment);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(Comment)
        if (character == LessThanSign)
        {
          _token.AppendToComment(LessThanSign);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CommentLessThanSign);
        }
        if (character == HyphenMinus)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CommentEndDash);
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          _token.AppendToComment(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(Comment);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInComment);
          return EmitCommentToken(false);
        }

        _token.AppendToComment(character);
        KRYS_ADVANCE_TO(Comment);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(CommentLessThanSign)
        if (character == ExclamationMark)
        {
          _token.AppendToComment(ExclamationMark);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CommentLessThanSignBang);
        }
        if (character == LessThanSign)
        {
          _token.AppendToComment(LessThanSign);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CommentLessThanSign);
        }

        KRYS_RECONSUME_IN(Comment);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(CommentLessThanSignBang)
        if (character == HyphenMinus)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CommentLessThanSignBangDash);
        }

        KRYS_RECONSUME_IN(Comment);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(CommentLessThanSignBangDash)
        if (character == HyphenMinus)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CommentLessThanSignBangDashDash);
        }

        KRYS_RECONSUME_IN(CommentEndDash);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(CommentLessThanSignBangDashDash)
        if (character == GreaterThanSign || character == EndOfFile)
        {
          KRYS_RECONSUME_IN(CommentEnd);
        }

        ParserError(ParseError::NestedComment);
        KRYS_RECONSUME_IN(CommentEnd);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(CommentEndDash)
        if (character == HyphenMinus)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CommentEnd);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInComment);
          return EmitCommentToken(false);
        }

        _token.AppendToComment(HyphenMinus);
        KRYS_RECONSUME_IN(Comment);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(CommentEnd)
        if (character == GreaterThanSign)
        {
          return EmitCommentToken(true);
        }
        if (character == ExclamationMark)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CommentEndBang);
        }
        if (character == HyphenMinus)
        {
          _token.AppendToComment(HyphenMinus);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CommentEnd);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInComment);
          return EmitCommentToken(false);
        }

        _token.AppendToComment("--");
        KRYS_RECONSUME_IN(Comment);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(CommentEndBang)
        if (character == HyphenMinus)
        {
          _token.AppendToComment("--!");
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CommentEndDash);
        }
        if (character == GreaterThanSign)
        {
          ParserError(ParseError::IncorrectlyClosedComment);
          return EmitCommentToken(true);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInComment);
          return EmitCommentToken(false);
        }

        _token.AppendToComment("--!");
        KRYS_RECONSUME_IN(Comment);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(DOCTYPE)
        if (IsTokenizerWhitespace(character))
        {
          KRYS_ADVANCE_TO(BeforeDOCTYPEName);
        }
        if (character == GreaterThanSign)
        {
          KRYS_RECONSUME_IN(BeforeDOCTYPEName);
        }
        if (character == EndOfFile)
        {
          ParserError(ParseError::EOFInDOCTYPE);
          _token.BeginDOCTYPE();
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(false);
        }

        ParserError(ParseError::MissingWhitespaceBeforeDOCTYPEName);
        KRYS_RECONSUME_IN(BeforeDOCTYPEName);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(BeforeDOCTYPEName)
        if (IsTokenizerWhitespace(character))
        {
          KRYS_ADVANCE_TO(BeforeDOCTYPEName);
        }
        if (krys::text::IsASCIIUpper(character))
        {
          _token.BeginDOCTYPE();
          _token.AppendToName(krys::text::ToASCIILowerUnchecked(character));
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEName);
        }
        if (character == Null)
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          _token.BeginDOCTYPE();
          _token.AppendToName(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEName);
        }
        if (character == GreaterThanSign)
        {
          ParserError(ParseError::MissingDOCTYPEName);
          _token.BeginDOCTYPE();
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(true);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInDOCTYPE);
          _token.BeginDOCTYPE();
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(false);
        }

        _token.BeginDOCTYPE();
        _token.AppendToName(character);
        KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEName);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(DOCTYPEName)
        if (IsTokenizerWhitespace(character))
        {
          KRYS_ADVANCE_TO(AfterDOCTYPEName);
        }
        if (character == GreaterThanSign)
        {
          return EmitDOCTYPEToken(true);
        }
        if (krys::text::IsASCIIUpper(character))
        {
          _token.AppendToName(krys::text::ToASCIILowerUnchecked(character));
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEName);
        }
        if (character == Null)
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          _token.AppendToName(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEName);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInDOCTYPE);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(false);
        }

        _token.AppendToName(character);
        KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEName);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(AfterDOCTYPEName)
        if (IsTokenizerWhitespace(character))
        {
          KRYS_ADVANCE_TO(AfterDOCTYPEName);
        }
        if (character == GreaterThanSign)
        {
          return EmitDOCTYPEToken(true);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInDOCTYPE);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(false);
        }
        if (krys::text::IsASCIICaselessEqual(character, 'p'))
        {
          auto result = _input.AdvancePast("public");
          if (result == InputStream::MatchResult::Matched)
          {
            KRYS_SWITCH_TO(AfterDOCTYPEPublicKeyword);
          }
          if (result == InputStream::MatchResult::NotEnoughCharacters)
          {
            KRYS_RETURN_IN_CURRENT_STATE(HasBufferedCharacterToken());
          }
        }
        if (krys::text::IsASCIICaselessEqual(character, 's'))
        {
          auto result = _input.AdvancePast("system");
          if (result == InputStream::MatchResult::Matched)
          {
            KRYS_SWITCH_TO(AfterDOCTYPESystemKeyword);
          }
          if (result == InputStream::MatchResult::NotEnoughCharacters)
          {
            KRYS_RETURN_IN_CURRENT_STATE(HasBufferedCharacterToken());
          }
        }

        ParserError(ParseError::InvalidCharacterSequenceAfterDOCTYPEName);
        _token.SetDOCTYPEForceQuirks();
        KRYS_RECONSUME_IN(BogusDOCTYPE);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(AfterDOCTYPEPublicKeyword)
        if (IsTokenizerWhitespace(character))
        {
          KRYS_ADVANCE_TO(BeforeDOCTYPEPublicIdentifier);
        }
        if (character == QuotationMark)
        {
          ParserError(ParseError::MissingWhitespaceAfterDOCTYPEPublicKeyword);
          _token.SetPublicIdentifierToEmptyString();
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEPublicIdentifierDoubleQuoted);
        }
        if (character == Apostrophe)
        {
          ParserError(ParseError::MissingWhitespaceAfterDOCTYPEPublicKeyword);
          _token.SetPublicIdentifierToEmptyString();
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEPublicIdentifierSingleQuoted);
        }
        if (character == GreaterThanSign)
        {
          ParserError(ParseError::MissingDOCTYPEPublicIdentifier);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(true);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInDOCTYPE);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(false);
        }

        ParserError(ParseError::MissingQuoteBeforeDOCTYPEPublicIdentifier);
        _token.SetDOCTYPEForceQuirks();
        KRYS_RECONSUME_IN(BogusDOCTYPE);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(BeforeDOCTYPEPublicIdentifier)
        if (IsTokenizerWhitespace(character))
        {
          KRYS_ADVANCE_TO(BeforeDOCTYPEPublicIdentifier);
        }
        if (character == QuotationMark)
        {
          _token.SetPublicIdentifierToEmptyString();
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEPublicIdentifierDoubleQuoted);
        }
        if (character == Apostrophe)
        {
          _token.SetPublicIdentifierToEmptyString();
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEPublicIdentifierSingleQuoted);
        }
        if (character == GreaterThanSign)
        {
          ParserError(ParseError::MissingDOCTYPEPublicIdentifier);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(true);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInDOCTYPE);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(false);
        }

        ParserError(ParseError::MissingQuoteBeforeDOCTYPEPublicIdentifier);
        _token.SetDOCTYPEForceQuirks();
        KRYS_RECONSUME_IN(BogusDOCTYPE);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(DOCTYPEPublicIdentifierDoubleQuoted)
        if (character == QuotationMark)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(AfterDOCTYPEPublicIdentifier);
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          _token.AppendToPublicIdentifier(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEPublicIdentifierDoubleQuoted);
        }
        if (character == GreaterThanSign)
        {
          ParserError(ParseError::AbruptDOCTYPEPublicIdentifier);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(true);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInDOCTYPE);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(false);
        }

        _token.AppendToPublicIdentifier(character);
        KRYS_ADVANCE_TO(DOCTYPEPublicIdentifierDoubleQuoted);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(DOCTYPEPublicIdentifierSingleQuoted)
        if (character == Apostrophe)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(AfterDOCTYPEPublicIdentifier);
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          _token.AppendToPublicIdentifier(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEPublicIdentifierSingleQuoted);
        }
        if (character == GreaterThanSign)
        {
          ParserError(ParseError::AbruptDOCTYPEPublicIdentifier);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(true);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInDOCTYPE);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(false);
        }

        _token.AppendToPublicIdentifier(character);
        KRYS_ADVANCE_TO(DOCTYPEPublicIdentifierSingleQuoted);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(AfterDOCTYPEPublicIdentifier)
        if (IsTokenizerWhitespace(character))
        {
          KRYS_ADVANCE_TO(BetweenDOCTYPEPublicAndSystemIdentifiers);
        }
        if (character == GreaterThanSign)
        {
          return EmitDOCTYPEToken(true);
        }
        if (character == QuotationMark)
        {
          ParserError(ParseError::MissingWhitespaceBetweenDOCTYPEPublicAndSystemIdentifiers);
          _token.SetSystemIdentifierToEmptyString();
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierDoubleQuoted);
        }
        if (character == Apostrophe)
        {
          ParserError(ParseError::MissingWhitespaceBetweenDOCTYPEPublicAndSystemIdentifiers);
          _token.SetSystemIdentifierToEmptyString();
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierSingleQuoted);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInDOCTYPE);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(false);
        }

        ParserError(ParseError::MissingQuoteBeforeDOCTYPESystemIdentifier);
        _token.SetDOCTYPEForceQuirks();
        KRYS_RECONSUME_IN(BogusDOCTYPE);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(BetweenDOCTYPEPublicAndSystemIdentifiers)
        if (IsTokenizerWhitespace(character))
        {
          KRYS_ADVANCE_TO(BetweenDOCTYPEPublicAndSystemIdentifiers);
        }
        if (character == GreaterThanSign)
        {
          return EmitDOCTYPEToken(true);
        }
        if (character == QuotationMark)
        {
          _token.SetSystemIdentifierToEmptyString();
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierDoubleQuoted);
        }
        if (character == Apostrophe)
        {
          _token.SetSystemIdentifierToEmptyString();
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierSingleQuoted);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInDOCTYPE);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(false);
        }

        ParserError(ParseError::MissingQuoteBeforeDOCTYPESystemIdentifier);
        _token.SetDOCTYPEForceQuirks();
        KRYS_RECONSUME_IN(BogusDOCTYPE);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(AfterDOCTYPESystemKeyword)
        if (IsTokenizerWhitespace(character))
        {
          KRYS_ADVANCE_TO(BeforeDOCTYPESystemIdentifier);
        }
        if (character == QuotationMark)
        {
          ParserError(ParseError::MissingWhitespaceAfterDOCTYPESystemKeyword);
          _token.SetSystemIdentifierToEmptyString();
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierDoubleQuoted);
        }
        if (character == Apostrophe)
        {
          ParserError(ParseError::MissingWhitespaceAfterDOCTYPESystemKeyword);
          _token.SetSystemIdentifierToEmptyString();
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierSingleQuoted);
        }
        if (character == GreaterThanSign)
        {
          ParserError(ParseError::MissingDOCTYPESystemIdentifier);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(true);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInDOCTYPE);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(false);
        }

        ParserError(ParseError::MissingQuoteBeforeDOCTYPESystemIdentifier);
        _token.SetDOCTYPEForceQuirks();
        KRYS_RECONSUME_IN(BogusDOCTYPE);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(BeforeDOCTYPESystemIdentifier)
        if (IsTokenizerWhitespace(character))
        {
          KRYS_ADVANCE_TO(BeforeDOCTYPESystemIdentifier);
        }
        if (character == QuotationMark)
        {
          _token.SetSystemIdentifierToEmptyString();
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierDoubleQuoted);
        }
        if (character == Apostrophe)
        {
          _token.SetSystemIdentifierToEmptyString();
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierSingleQuoted);
        }
        if (character == GreaterThanSign)
        {
          ParserError(ParseError::MissingDOCTYPESystemIdentifier);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(true);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInDOCTYPE);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(false);
        }

        ParserError(ParseError::MissingQuoteBeforeDOCTYPESystemIdentifier);
        _token.SetDOCTYPEForceQuirks();
        KRYS_RECONSUME_IN(BogusDOCTYPE);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(DOCTYPESystemIdentifierDoubleQuoted)
        if (character == QuotationMark)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(AfterDOCTYPESystemIdentifier);
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          _token.AppendToSystemIdentifier(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierDoubleQuoted);
        }
        if (character == GreaterThanSign)
        {
          ParserError(ParseError::AbruptDOCTYPESystemIdentifier);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(true);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInDOCTYPE);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(false);
        }

        _token.AppendToSystemIdentifier(character);
        KRYS_ADVANCE_TO(DOCTYPESystemIdentifierDoubleQuoted);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(DOCTYPESystemIdentifierSingleQuoted)
        if (character == Apostrophe)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(AfterDOCTYPESystemIdentifier);
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          _token.AppendToSystemIdentifier(Replacement);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierSingleQuoted);
        }
        if (character == GreaterThanSign)
        {
          ParserError(ParseError::AbruptDOCTYPESystemIdentifier);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(true);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInDOCTYPE);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(false);
        }

        _token.AppendToSystemIdentifier(character);
        KRYS_ADVANCE_TO(DOCTYPESystemIdentifierSingleQuoted);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(AfterDOCTYPESystemIdentifier)
        if (IsTokenizerWhitespace(character))
        {
          KRYS_ADVANCE_TO(AfterDOCTYPESystemIdentifier);
        }
        if (character == GreaterThanSign)
        {
          return EmitDOCTYPEToken(true);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInDOCTYPE);
          _token.SetDOCTYPEForceQuirks();
          return EmitDOCTYPEToken(false);
        }

        ParserError(ParseError::UnexpectedCharacterAfterDOCTYPESystemIdentifier);
        KRYS_RECONSUME_IN(BogusDOCTYPE);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(BogusDOCTYPE)
        if (character == GreaterThanSign)
        {
          return EmitDOCTYPEToken(true);
        }
        if (character == Null) KRYS_UNLIKELY
        {
          ParserError(ParseError::UnexpectedNullCharacter);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(BogusDOCTYPE);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          return EmitDOCTYPEToken(false);
        }

        KRYS_ADVANCE_TO(BogusDOCTYPE);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(CDATASection)
        if (character == RightSquareBracket)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CDATASectionBracket);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInCDATA);
          KRYS_RECONSUME_IN(Data);
        }

        _token.AppendToCharacters(character);
        KRYS_ADVANCE_TO(CDATASection);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(CDATASectionBracket)
        if (character == RightSquareBracket)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CDATASectionEnd);
        }

        _token.AppendToCharacters(RightSquareBracket);
        KRYS_RECONSUME_IN(CDATASection);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(CDATASectionEnd)
        if (character == RightSquareBracket)
        {
          _token.AppendToCharacters(RightSquareBracket);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(CDATASectionEnd);
        }
        if (character == GreaterThanSign)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(Data);
        }

        _token.AppendToCharacters(RightSquareBracket);
        _token.AppendToCharacters(RightSquareBracket);
        KRYS_RECONSUME_IN(CDATASection);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ProcessingInstructionOpen)
        if (krys::text::IsASCIIAlpha(character) || character == LowLine)
        {
          KRYS_RECONSUME_IN(ProcessingInstructionTarget);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInProcessingInstruction);
          return EmitEOFToken();
        }

        ParserError(ParseError::InvalidFirstCharacterOfProcessingInstructionTarget);
        ConvertTemporaryBufferToComment();
        KRYS_RECONSUME_IN(BogusComment);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ProcessingInstructionTarget)
        if (IsTokenizerWhitespace(character) || character == QuestionMark || character == GreaterThanSign)
        {
          if (IsTemporaryBufferASCIICaseInsensitiveMatch(U"xml")
              || IsTemporaryBufferASCIICaseInsensitiveMatch(U"xml-stylesheet"))
          {
            ParserError(ParseError::DisallowedProcessingInstructionTarget);
            ConvertTemporaryBufferToComment();
            KRYS_RECONSUME_IN(BogusComment);
          }

          _token.BeginProcessingInstruction(_temporaryBuffer);
          KRYS_RECONSUME_IN(AfterProcessingInstructionTarget);
        }
        if (krys::text::IsASCIIAlphanumeric(character) || character == HyphenMinus || character == LowLine)
        {
          _temporaryBuffer.push_back(character);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ProcessingInstructionTarget);
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInProcessingInstruction);
          return EmitEOFToken();
        }

        ParserError(ParseError::InvalidProcessingInstructionTarget);
        ConvertTemporaryBufferToComment();
        KRYS_RECONSUME_IN(BogusComment);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(AfterProcessingInstructionTarget)
        if (IsTokenizerWhitespace(character))
        {
          KRYS_ADVANCE_TO(AfterProcessingInstructionTarget);
        }

        KRYS_RECONSUME_IN(ProcessingInstructionData);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ProcessingInstructionData)
        if (character == QuestionMark)
        {
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(ProcessingInstructionQuestionable);
        }
        if (character == GreaterThanSign)
        {
          return EmitProcessingInstructionToken();
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInProcessingInstruction);
          return EmitEOFToken();
        }

        _token.AppendToProcessingInstructionData(character);
        KRYS_ADVANCE_TO(ProcessingInstructionData);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(ProcessingInstructionQuestionable)
        if (character == GreaterThanSign)
        {
          return EmitProcessingInstructionToken();
        }
        if (character == EndOfFile) KRYS_UNLIKELY
        {
          ParserError(ParseError::EOFInProcessingInstruction);
          return EmitEOFToken();
        }

        _token.AppendToProcessingInstructionData(QuestionMark);
        KRYS_RECONSUME_IN(ProcessingInstructionData);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(CharacterReference)
        _temporaryBuffer.clear();
        _temporaryBuffer.push_back(Ampersand);

        if (krys::text::IsASCIIAlphanumeric(character))
        {
          _temporaryBuffer.push_back(character);
          _namedCharacterReferenceMatchEntries = SearchNamedCharacterReferences(_temporaryBuffer);
          _longestCharacterReferenceMatch = nullptr;
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(NamedCharacterReference);
        }
        if (character == NumberSign)
        {
          _temporaryBuffer.push_back(character);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(NumericCharacterReference);
        }

        FlushCodePointsConsumedAsACharacterReference();
        KRYS_RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE();
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(NamedCharacterReference)
        _temporaryBuffer.push_back(character);
        _namedCharacterReferenceMatchEntries =
          SearchNamedCharacterReferences(_temporaryBuffer, _namedCharacterReferenceMatchEntries);

        if (!_namedCharacterReferenceMatchEntries.empty())
        {
          const auto &first = _namedCharacterReferenceMatchEntries[0];
          if (character == Semicolon) // Exact match possible only if semicolon is present
          {
            _temporaryBuffer.clear();
            AppendToTemporaryBuffer(first.ToSpan());
            FlushCodePointsConsumedAsACharacterReference();
            KRYS_ADVANCE_TO_CHARACTER_REFERENCE_RETURN_STATE();
          }

          // Keep track of the longest match so far
          if (first.Name.size() == _temporaryBuffer.size())
          {
            _longestCharacterReferenceMatch = &first;
          }

          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(NamedCharacterReference);
        }

        if (_longestCharacterReferenceMatch)
        {
          const auto &match = *_longestCharacterReferenceMatch;

          List<char32> nonMatchingCharacters;
          for (size_t i = match.Name.size(); i < _temporaryBuffer.size() - 1; ++i)
          {
            nonMatchingCharacters.push_back(_temporaryBuffer[i]);
          }
          _temporaryBuffer.clear();

          if (CharacterReferenceWasConsumedAsPartOfAnAttribute() && match.Name.back() != Semicolon
              && (character == EqualSign || krys::text::IsASCIIAlphanumeric(character)))
          {
            AppendToTemporaryBuffer(match.Name);
            AppendToTemporaryBuffer(nonMatchingCharacters);
            FlushCodePointsConsumedAsACharacterReference();
            KRYS_RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE();
          }

          AppendToTemporaryBuffer(match.ToSpan());
          AppendToTemporaryBuffer(nonMatchingCharacters);
          FlushCodePointsConsumedAsACharacterReference();

          if (match.Name.back() != Semicolon)
          {
            ParserError(ParseError::MissingSemicolonAfterCharacterReference);
          }

          KRYS_RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE();
        }

        // no matches at all
        _temporaryBuffer.pop_back(); // remove the last character added so we can reconsume it
        FlushCodePointsConsumedAsACharacterReference();
        KRYS_RECONSUME_IN(AmbiguousAmpersand);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(AmbiguousAmpersand)
        if (krys::text::IsASCIIAlphanumeric(character))
        {
          if (CharacterReferenceWasConsumedAsPartOfAnAttribute())
          {
            _token.AppendToCurrentAttributeValue(character);
          }
          else
          {
            _token.AppendToCharacters(character);
          }
          KRYS_ADVANCE_TO(AmbiguousAmpersand);
        }
        if (character == Semicolon)
        {
          ParserError(ParseError::UnknownNamedCharacterReference);
          KRYS_RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE();
        }

        KRYS_RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE();
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(NumericCharacterReference)
        _characterReferenceCode = 0;
        if (krys::text::IsASCIICaselessEqual(character, 'x'))
        {
          _temporaryBuffer.push_back(character);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(HexadecimalCharacterReferenceStart);
        }

        KRYS_RECONSUME_IN(DecimalCharacterReferenceStart);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(HexadecimalCharacterReferenceStart)
        if (krys::text::IsASCIIHexDigit(character))
        {
          KRYS_RECONSUME_IN(HexadecimalCharacterReference);
        }

        ParserError(ParseError::AbsenceOfDigitsInNumericCharacterReference);
        FlushCodePointsConsumedAsACharacterReference();
        KRYS_RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE();
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(DecimalCharacterReferenceStart)
        if (krys::text::IsASCIIDigit(character))
        {
          KRYS_RECONSUME_IN(DecimalCharacterReference);
        }

        ParserError(ParseError::AbsenceOfDigitsInNumericCharacterReference);
        FlushCodePointsConsumedAsACharacterReference();
        KRYS_RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE();
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(HexadecimalCharacterReference)
        if (krys::text::IsASCIIHexDigit(character))
        {
          // Anything past 0x10FFFF is outside the Unicode range, so the code stops growing there rather
          // than overflowing back into it.
          _characterReferenceCode = std::min<int64>(
            _characterReferenceCode * 16 + static_cast<int64>(krys::text::ToASCIIHexValue(character)),
            MaxCharacterReferenceCode);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(HexadecimalCharacterReference);
        }
        if (character != Semicolon)
        {
          ParserError(ParseError::MissingSemicolonAfterCharacterReference);
        }

        KRYS_RECONSUME_IN(NumericCharacterReferenceEnd);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(DecimalCharacterReference)
        if (krys::text::IsASCIIDigit(character))
        {
          _characterReferenceCode = std::min<int64>(
            _characterReferenceCode * 10 + static_cast<int64>(character - '0'), MaxCharacterReferenceCode);
          KRYS_ADVANCE_PAST_NON_NEWLINE_TO(DecimalCharacterReference);
        }
        if (character != Semicolon)
        {
          ParserError(ParseError::MissingSemicolonAfterCharacterReference);
        }

        KRYS_RECONSUME_IN(NumericCharacterReferenceEnd);
        KRYS_END_STATE()

        KRYS_BEGIN_STATE(NumericCharacterReferenceEnd)
        if (_characterReferenceCode == Null)
        {
          ParserError(ParseError::NullCharacterReference);
          _characterReferenceCode = Replacement;
        }
        else if (_characterReferenceCode > 0x10FFFF)
        {
          ParserError(ParseError::CharacterReferenceOutsideUnicodeRange);
          _characterReferenceCode = Replacement;
        }
        else if (krys::text::IsSurrogate(static_cast<char32>(_characterReferenceCode)))
        {
          ParserError(ParseError::SurrogateCharacterReference);
          _characterReferenceCode = Replacement;
        }
        else if (krys::text::IsNonUnicodeCharacter(static_cast<char32>(_characterReferenceCode)))
        {
          ParserError(ParseError::NonCharacterCharacterReference);
        }
        else if (_characterReferenceCode == '\x0D'
                 || krys::text::IsASCIINonWhitespaceControl(static_cast<char32>(_characterReferenceCode)))
        {
          ParserError(ParseError::ControlCharacterReference);
        }
        else
        {
          auto lookup = SearchNumericCharacterReferences(static_cast<char32>(_characterReferenceCode));
          if (lookup != 0)
          {
            _characterReferenceCode = lookup;
          }
        }

        _temporaryBuffer.clear();
        _temporaryBuffer.push_back(static_cast<char32>(_characterReferenceCode));
        FlushCodePointsConsumedAsACharacterReference();

        if (character == Semicolon)
        {
          KRYS_ADVANCE_TO_CHARACTER_REFERENCE_RETURN_STATE();
        }

        KRYS_RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE();
        KRYS_END_STATE()
      }

      std::unreachable();
      return false;
    }

#pragma warning(pop)

#undef KRYS_BEGIN_STATE
#undef KRYS_END_STATE
#undef KRYS_RETURN_IN_CURRENT_STATE
#undef KRYS_RETURN_IN_CURRENT_STATE_IF_CHARACTERS_BUFFERED
#undef KRYS_RECONSUME_IN
#undef KRYS_ADVANCE_TO
#undef KRYS_ADVANCE_PAST_NON_NEWLINE_TO
#undef KRYS_RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE
#undef KRYS_ADVANCE_TO_CHARACTER_REFERENCE_RETURN_STATE
#undef KRYS_SWITCH_TO
  };
}
