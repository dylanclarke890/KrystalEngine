#pragma once

#include "Krystal.HTML/HTML/Enums/HTMLParseError.hpp"
#include "Krystal.HTML/HTML/Enums/TokenizerState.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLCharacterReference.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLInputStream.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLToken.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/ReferenceWrapper.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"
#include "Krystal.Text/ASCII.hpp"
#include "Krystal.Text/StringConversion.hpp"
#include "Krystal.Text/Unicode.hpp"
#include <ranges>

namespace Krys::HTML
{
  class NextTokenPtr
  {
  private:
    RawPtr<HTMLToken> _token {nullptr};

  public:
    NextTokenPtr() noexcept = default;
    NextTokenPtr &operator=(NextTokenPtr &&) = delete;

    ~NextTokenPtr() noexcept
    {
      if (_token != nullptr)
      {
        _token->Clear();
      }
    }

    NextTokenPtr(NextTokenPtr &&other) noexcept : _token(std::exchange(other._token, nullptr))
    {
    }

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

    HTMLToken &operator*() const noexcept
    {
      assert(_token != nullptr);
      return *_token;
    }

    RawPtr<HTMLToken> operator->() const noexcept
    {
      assert(_token != nullptr);
      return _token;
    }

  private:
    friend class HTMLTokenizer;

    explicit NextTokenPtr(RawPtr<HTMLToken> token) noexcept : _token(token)
    {
    }
  };

  class HTMLTokenizer
  {
  public:
    struct HTMLTokenizerError
    {
      HTMLParseError Error;
      HTMLInputStream::SourceLocation Location {};
    };

  private:
    TokenizerState _state : BitCount<TokenizerState>() {TokenizerState::Data};
    TokenizerState _characterReferenceReturnState : BitCount<TokenizerState>() {TokenizerState::Data};
    HTMLToken _token;
    HTMLInputStream &_input;
    SmallList<char32, 32u> _appropriateEndTagName;
    SmallList<char32, 32u> _bufferedEndTagName;
    bool _isCDATASectionAllowed : 1 {false};

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#character-reference-code
    int64 _characterReferenceCode {0};

    /// @see https://html.spec.whatwg.org/#temporary-buffer
    SmallList<char32, 32u> _temporaryBuffer;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#named-character-reference-state
    Span<const NamedCharacterReferenceEntry> _namedCharacterReferenceMatchEntries;
    const NamedCharacterReferenceEntry *_longestCharacterReferenceMatch {nullptr};

    List<HTMLTokenizerError> _parseErrors;

  public:
    HTMLTokenizer(HTMLInputStream &input) noexcept : _input(input)
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

    KRYS_NODISCARD const List<HTMLTokenizerError> &ParseErrors() const noexcept
    {
      return _parseErrors;
    }

    void IsCDATAAllowed(bool isAllowed) noexcept
    {
      _isCDATASectionAllowed = isAllowed;
    }

  private:
    KRYS_NODISCARD bool ProcessToken() noexcept
    {
      if (!_input.Peek())
      {
        return HasBufferedCharacterToken();
      }

      return StepTokenizationStateMachine();
    }

    KRYS_NODISCARD bool HasBufferedCharacterToken() const noexcept
    {
      return _token.Type() == HTMLTokenType::Character;
    }

    KRYS_NODISCARD bool IsCDATAAllowed() const noexcept
    {
      return _isCDATASectionAllowed;
    }

    void AppendToTemporaryBuffer(const auto &characters)
    {
      _temporaryBuffer.append(characters.begin(), characters.end());
    }

    void BufferCharacter(char32 character) noexcept
    {
      assert(character != HTMLInputStream::EOFMarker);
      _token.AppendToCharacters(character);
    }

    template <size_t N>
    void BufferCharacters(Array<char32, N> characters) noexcept
    {
#if KRYS_ENV(DEV)
      for (auto character : characters)
      {
        assert(character != HTMLInputStream::EOFMarker);
      }
#endif
      _token.AppendToCharacters(characters);
    }

    void BufferCharacters(Span<char32> characters) noexcept
    {
#if KRYS_ENV(DEV)
      for (auto character : characters)
      {
        assert(character != HTMLInputStream::EOFMarker);
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
        ParserError(HTMLParseError::DuplicateAttribute);
        attributes.pop_back();
      }

      _token.EndAttribute();
    }

    void ParserError(HTMLParseError error) noexcept
    {
      _parseErrors.push_back(HTMLTokenizerError {
        .Error = error,
        .Location = _input.GetCurrentLocation(),
      });
    }

    KRYS_NODISCARD bool IsTokenizerWhitespace(char32 character) const noexcept
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

    void BeginEndTagUsingBuffer() noexcept
    {
      _token.BeginEndTag(_bufferedEndTagName);
      _bufferedEndTagName.clear();
      _appropriateEndTagName.clear();
      _temporaryBuffer.clear();
    }

    KRYS_NODISCARD bool CommitToPartialEndTag(char32 character, TokenizerState state) noexcept
    {
      assert(_input.NextInputCharacter() == character);
      _temporaryBuffer.push_back(character);
      _input.Advance();

      if (HasBufferedCharacterToken())
      {
        // Emit the buffered token so that on the next pass we can continue parsing the end tag.
        _state = state;
        return true;
      }

      BeginEndTagUsingBuffer();
      return false;
    }

    KRYS_NODISCARD bool CommitToCompleteEndTag() noexcept
    {
      assert(_input.NextInputCharacter() == U'>');
      _temporaryBuffer.push_back(U'>');

      _input.Advance();
      _state = TokenizerState::Data;

      if (HasBufferedCharacterToken())
      {
        // Emit the character token so that on the next pass we can emit the end tag.
        return true;
      }

      BeginEndTagUsingBuffer();
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
      assert(_token.Type() == HTMLTokenType::StartTag || _token.Type() == HTMLTokenType::EndTag);
      if (_token.Type() == HTMLTokenType::StartTag)
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
      assert(_token.Type() == HTMLTokenType::DOCTYPE);
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
      assert(_token.Type() == HTMLTokenType::Comment);
      _state = TokenizerState::Data;

      if (!consumeCurrentCharacter)
      {
        return true;
      }

      _input.Advance();
      return true;
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

#define BEGIN_STATE(stateName)                                                                               \
  case TokenizerState::stateName:                                                                            \
  stateName:                                                                                                 \
  {                                                                                                          \
    constexpr auto currentState = TokenizerState::stateName;
#define END_STATE()                                                                                          \
  assert(false);                                                                                             \
  break;                                                                                                     \
  }

#define RETURN_IN_CURRENT_STATE(expression)                                                                  \
  do                                                                                                         \
  {                                                                                                          \
    _state = currentState;                                                                                   \
    return expression;                                                                                       \
  } while (false)

#define RETURN_IN_CURRENT_STATE_IF_CHARACTERS_BUFFERED()                                                     \
  do                                                                                                         \
  {                                                                                                          \
    if (HasBufferedCharacterToken())                                                                         \
    {                                                                                                        \
      _state = currentState;                                                                                 \
      return true;                                                                                           \
    }                                                                                                        \
  } while (false)

// For when the HTML spec says "reconsume the current input character in the <mumble> state."
#define RECONSUME_IN(newState)                                                                               \
  do                                                                                                         \
  {                                                                                                          \
    goto newState;                                                                                           \
  } while (false)

// For when the HTML spec says "consume the next input character ... and switch to the <mumble> state."
#define ADVANCE_TO(newState)                                                                                 \
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
#define ADVANCE_PAST_NON_NEWLINE_TO(newState)                                                                \
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
#define SWITCH_TO(newState)                                                                                  \
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

#define RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE()                                                      \
  do                                                                                                         \
  {                                                                                                          \
    if (_characterReferenceReturnState == TokenizerState::Data)                                              \
    {                                                                                                        \
      RECONSUME_IN(Data);                                                                                    \
    }                                                                                                        \
    if (_characterReferenceReturnState == TokenizerState::RCDATA)                                            \
    {                                                                                                        \
      RECONSUME_IN(RCDATA);                                                                                  \
    }                                                                                                        \
    if (_characterReferenceReturnState == TokenizerState::AttributeValueDoubleQuoted)                        \
    {                                                                                                        \
      RECONSUME_IN(AttributeValueDoubleQuoted);                                                              \
    }                                                                                                        \
    if (_characterReferenceReturnState == TokenizerState::AttributeValueSingleQuoted)                        \
    {                                                                                                        \
      RECONSUME_IN(AttributeValueSingleQuoted);                                                              \
    }                                                                                                        \
    if (_characterReferenceReturnState == TokenizerState::AttributeValueUnquoted)                            \
    {                                                                                                        \
      RECONSUME_IN(AttributeValueUnquoted);                                                                  \
    }                                                                                                        \
  } while (false)

#define ADVANCE_TO_CHARACTER_REFERENCE_RETURN_STATE()                                                        \
  do                                                                                                         \
  {                                                                                                          \
    if (_characterReferenceReturnState == TokenizerState::Data)                                              \
    {                                                                                                        \
      ADVANCE_TO(Data);                                                                                      \
    }                                                                                                        \
    if (_characterReferenceReturnState == TokenizerState::RCDATA)                                            \
    {                                                                                                        \
      ADVANCE_TO(RCDATA);                                                                                    \
    }                                                                                                        \
    if (_characterReferenceReturnState == TokenizerState::AttributeValueDoubleQuoted)                        \
    {                                                                                                        \
      ADVANCE_TO(AttributeValueDoubleQuoted);                                                                \
    }                                                                                                        \
    if (_characterReferenceReturnState == TokenizerState::AttributeValueSingleQuoted)                        \
    {                                                                                                        \
      ADVANCE_TO(AttributeValueSingleQuoted);                                                                \
    }                                                                                                        \
    if (_characterReferenceReturnState == TokenizerState::AttributeValueUnquoted)                            \
    {                                                                                                        \
      ADVANCE_TO(AttributeValueUnquoted);                                                                    \
    }                                                                                                        \
  } while (false)

#pragma endregion

    /// @see https://html.spec.whatwg.org/#tokenization
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
      constexpr char32 EqualSign = U'=';
      constexpr char32 QuotationMark = U'"';
      constexpr char32 Apostrophe = U'\'';
      constexpr char32 GraveAccent = U'`';
      constexpr char32 RightSquareBracket = U']';
      constexpr char32 NumberSign = U'#';
      constexpr char32 Semicolon = U';';
      constexpr char32 EndOfFile = HTMLInputStream::EOFMarker;
      constexpr char32 Replacement = Krys::Text::Unicode::Replacement<char32>;

      char32 character = _input.NextInputCharacter();
      switch (_state)
      {
        BEGIN_STATE(Data)
          if (character == Ampersand)
          {
            _characterReferenceReturnState = TokenizerState::Data;
            ADVANCE_PAST_NON_NEWLINE_TO(CharacterReference);
          }
          if (character == LessThanSign)
          {
            RETURN_IN_CURRENT_STATE_IF_CHARACTERS_BUFFERED();
            ADVANCE_PAST_NON_NEWLINE_TO(TagOpen);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            RETURN_IN_CURRENT_STATE_IF_CHARACTERS_BUFFERED();
            return EmitEOFToken();
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            BufferCharacter(Null); // parser error but we still emit the null as per the spec.
            ADVANCE_PAST_NON_NEWLINE_TO(Data);
          }

          BufferCharacter(character);
          ADVANCE_TO(Data);
        END_STATE()

        BEGIN_STATE(RCDATA)
          if (character == Ampersand)
          {
            _characterReferenceReturnState = TokenizerState::RCDATA;
            ADVANCE_PAST_NON_NEWLINE_TO(CharacterReference);
          }
          if (character == LessThanSign)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(RCDATALessThanSign);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            RECONSUME_IN(Data);
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            BufferCharacter(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(RCDATA);
          }

          BufferCharacter(character);
          ADVANCE_TO(RCDATA);
        END_STATE()

        BEGIN_STATE(RAWTEXT)
          if (character == LessThanSign)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(RAWTEXTLessThanSign);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            RECONSUME_IN(Data);
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            BufferCharacter(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(RAWTEXT);
          }

          BufferCharacter(character);
          ADVANCE_TO(RAWTEXT);
        END_STATE()

        BEGIN_STATE(ScriptData)
          if (character == LessThanSign)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataLessThanSign);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            RECONSUME_IN(Data);
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            BufferCharacter(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptData);
          }

          BufferCharacter(character);
          ADVANCE_TO(ScriptData);
        END_STATE()

        BEGIN_STATE(PLAINTEXT)
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            RECONSUME_IN(Data);
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            BufferCharacter(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(PLAINTEXT);
          }

          BufferCharacter(character);
          ADVANCE_TO(PLAINTEXT);
        END_STATE()

        BEGIN_STATE(TagOpen)
          if (character == ExclamationMark)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(MarkupDeclarationOpen);
          }
          if (character == Solidus)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(EndTagOpen);
          }
          if (Krys::Text::IsASCIIAlpha(character))
          {
            _token.BeginStartTag(Krys::Text::ToASCIILower(character));
            ADVANCE_PAST_NON_NEWLINE_TO(TagName);
          }
          if (character == QuestionMark)
          {
            ParserError(HTMLParseError::UnexpectedQuestionMarkInsteadOfTagName);
            _token.BeginComment();
            RECONSUME_IN(BogusComment);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFBeforeTagName);
            BufferCharacter(LessThanSign);
            RECONSUME_IN(Data);
          }

          ParserError(HTMLParseError::InvalidFirstCharacterOfTagName);
          BufferCharacter(LessThanSign);
          RECONSUME_IN(Data);
        END_STATE()

        BEGIN_STATE(EndTagOpen)
          if (Krys::Text::IsASCIIAlpha(character))
          {
            _token.BeginEndTag(Krys::Text::ToASCIILower(character));
            ADVANCE_PAST_NON_NEWLINE_TO(TagName);
          }
          if (character == GreaterThanSign)
          {
            ParserError(HTMLParseError::MissingEndTagName);
            ADVANCE_PAST_NON_NEWLINE_TO(Data); // We don't emit the greater than sign, as per the spec.
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFBeforeTagName);
            BufferCharacters(Array {LessThanSign, Solidus});
            RECONSUME_IN(Data);
          }

          ParserError(HTMLParseError::InvalidFirstCharacterOfTagName);
          _token.BeginComment();
          RECONSUME_IN(BogusComment);
        END_STATE()

        BEGIN_STATE(TagName)
          if (IsTokenizerWhitespace(character))
          {
            ADVANCE_TO(BeforeAttributeName);
          }
          if (character == Solidus)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(SelfClosingStartTag);
          }
          if (character == GreaterThanSign)
          {
            return EmitTagToken();
          }
          if (Krys::Text::IsASCIIUpper(character))
          {
            _token.AppendToName(Krys::Text::ToASCIILowerUnchecked(character));
            ADVANCE_PAST_NON_NEWLINE_TO(TagName);
          }
          if (character == Null)
          {
            _token.AppendToName(Replacement);
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            ADVANCE_PAST_NON_NEWLINE_TO(TagName);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInTag);
            RECONSUME_IN(Data);
          }

          _token.AppendToName(character);
          ADVANCE_PAST_NON_NEWLINE_TO(TagName);

        END_STATE()

        BEGIN_STATE(RCDATALessThanSign)
          if (character == Solidus)
          {
            _temporaryBuffer.clear();
            assert(_bufferedEndTagName.empty());
            ADVANCE_PAST_NON_NEWLINE_TO(RCDATAEndTagOpen);
          }

          BufferCharacter(LessThanSign);
          RECONSUME_IN(RCDATA);
        END_STATE()

        BEGIN_STATE(RCDATAEndTagOpen)
          if (Krys::Text::IsASCIIAlpha(character))
          {
            _temporaryBuffer.push_back(character);
            AppendToPossibleEndTag(Krys::Text::ToASCIILower(character));
            ADVANCE_PAST_NON_NEWLINE_TO(RCDATAEndTagName);
          }

          BufferCharacters(Array {LessThanSign, Solidus});
          RECONSUME_IN(RCDATA);
        END_STATE()

        BEGIN_STATE(RCDATAEndTagName)
          if (IsTokenizerWhitespace(character))
          {
            if (IsAppropriateEndTag())
            {
              if (CommitToPartialEndTag(character, TokenizerState::BeforeAttributeName))
              {
                return true;
              }
            }
            SWITCH_TO(BeforeAttributeName);
          }
          if (character == Solidus)
          {
            if (IsAppropriateEndTag())
            {
              if (CommitToPartialEndTag(Solidus, TokenizerState::SelfClosingStartTag))
              {
                return true;
              }
            }
            SWITCH_TO(SelfClosingStartTag);
          }
          if (character == GreaterThanSign)
          {
            if (IsAppropriateEndTag())
            {
              return CommitToCompleteEndTag();
            }
          }
          if (Krys::Text::IsASCIIAlpha(character))
          {
            _temporaryBuffer.push_back(character);
            AppendToPossibleEndTag(Krys::Text::ToASCIILower(character));
            ADVANCE_PAST_NON_NEWLINE_TO(RCDATAEndTagName);
          }

          BufferCharacters(Array {LessThanSign, Solidus});
          BufferCharacters(_temporaryBuffer);
          _bufferedEndTagName.clear();
          _temporaryBuffer.clear();
          RECONSUME_IN(RCDATA);
        END_STATE()

        BEGIN_STATE(RAWTEXTLessThanSign)
          if (character == Solidus)
          {
            _temporaryBuffer.clear();
            assert(_bufferedEndTagName.empty());
            ADVANCE_PAST_NON_NEWLINE_TO(RAWTEXTEndTagOpen);
          }

          BufferCharacter(LessThanSign);
          RECONSUME_IN(RAWTEXT);
        END_STATE()

        BEGIN_STATE(RAWTEXTEndTagOpen)
          if (Krys::Text::IsASCIIAlpha(character))
          {
            _temporaryBuffer.push_back(character);
            AppendToPossibleEndTag(Krys::Text::ToASCIILower(character));
            ADVANCE_PAST_NON_NEWLINE_TO(RAWTEXTEndTagName);
          }

          BufferCharacters(Array {LessThanSign, Solidus});
          RECONSUME_IN(RAWTEXT);
        END_STATE()

        BEGIN_STATE(RAWTEXTEndTagName)
          if (IsTokenizerWhitespace(character))
          {
            if (IsAppropriateEndTag())
            {
              if (CommitToPartialEndTag(character, TokenizerState::BeforeAttributeName))
              {
                return true;
              }
            }
            SWITCH_TO(BeforeAttributeName);
          }
          if (character == Solidus)
          {
            if (IsAppropriateEndTag())
            {
              if (CommitToPartialEndTag(Solidus, TokenizerState::SelfClosingStartTag))
              {
                return true;
              }
            }
            SWITCH_TO(SelfClosingStartTag);
          }
          if (character == GreaterThanSign)
          {
            if (IsAppropriateEndTag())
            {
              return CommitToCompleteEndTag();
            }
          }
          if (Krys::Text::IsASCIIAlpha(character))
          {
            _temporaryBuffer.push_back(character);
            AppendToPossibleEndTag(Krys::Text::ToASCIILower(character));
            ADVANCE_PAST_NON_NEWLINE_TO(RAWTEXTEndTagName);
          }

          BufferCharacters(Array {LessThanSign, Solidus});
          BufferCharacters(_temporaryBuffer);
          _bufferedEndTagName.clear();
          _temporaryBuffer.clear();
          RECONSUME_IN(RAWTEXT);
        END_STATE()

        BEGIN_STATE(ScriptDataLessThanSign)
          if (character == Solidus)
          {
            _temporaryBuffer.clear();
            assert(_bufferedEndTagName.empty());
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEndTagOpen);
          }
          if (character == ExclamationMark)
          {
            BufferCharacters(Array {LessThanSign, ExclamationMark});
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapeStart);
          }

          BufferCharacter(LessThanSign);
          RECONSUME_IN(ScriptData);
        END_STATE()

        BEGIN_STATE(ScriptDataEndTagOpen)
          if (Krys::Text::IsASCIIAlpha(character))
          {
            _temporaryBuffer.push_back(character);
            AppendToPossibleEndTag(Krys::Text::ToASCIILower(character));
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEndTagName);
          }

          BufferCharacters(Array {LessThanSign, Solidus});
          RECONSUME_IN(ScriptData);
        END_STATE()

        BEGIN_STATE(ScriptDataEndTagName)
          if (IsTokenizerWhitespace(character))
          {
            if (IsAppropriateEndTag())
            {
              if (CommitToPartialEndTag(character, TokenizerState::BeforeAttributeName))
              {
                return true;
              }
            }
            SWITCH_TO(BeforeAttributeName);
          }
          if (character == Solidus)
          {
            if (IsAppropriateEndTag())
            {
              if (CommitToPartialEndTag(Solidus, TokenizerState::SelfClosingStartTag))
              {
                return true;
              }
            }
            SWITCH_TO(SelfClosingStartTag);
          }
          if (character == GreaterThanSign)
          {
            if (IsAppropriateEndTag())
            {
              return CommitToCompleteEndTag();
            }
          }
          if (Krys::Text::IsASCIIAlpha(character))
          {
            _temporaryBuffer.push_back(character);
            AppendToPossibleEndTag(Krys::Text::ToASCIILower(character));
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEndTagName);
          }

          BufferCharacters(Array {LessThanSign, Solidus});
          BufferCharacters(_temporaryBuffer);
          _bufferedEndTagName.clear();
          _temporaryBuffer.clear();
          RECONSUME_IN(ScriptData);
        END_STATE()

        BEGIN_STATE(ScriptDataEscapeStart)
          if (character == HyphenMinus)
          {
            BufferCharacter(HyphenMinus);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapeStartDash);
          }

          RECONSUME_IN(ScriptData);
        END_STATE()

        BEGIN_STATE(ScriptDataEscapeStartDash)
          if (character == HyphenMinus)
          {
            BufferCharacter(HyphenMinus);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedDashDash);
          }

          RECONSUME_IN(ScriptData);
        END_STATE()

        BEGIN_STATE(ScriptDataEscaped)
          if (character == HyphenMinus)
          {
            BufferCharacter(HyphenMinus);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedDash);
          }
          if (character == LessThanSign)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedLessThanSign);
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            BufferCharacter(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscaped);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInScriptHTMLCommentLikeText);
            RECONSUME_IN(Data);
          }

          BufferCharacter(character);
          ADVANCE_TO(ScriptDataEscaped);
        END_STATE()

        BEGIN_STATE(ScriptDataEscapedDash)
          if (character == HyphenMinus)
          {
            BufferCharacter(HyphenMinus);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedDashDash);
          }
          if (character == LessThanSign)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedLessThanSign);
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            BufferCharacter(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscaped);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInScriptHTMLCommentLikeText);
            RECONSUME_IN(Data);
          }

          BufferCharacter(character);
          ADVANCE_TO(ScriptDataEscaped);
        END_STATE()

        BEGIN_STATE(ScriptDataEscapedDashDash)
          if (character == HyphenMinus)
          {
            BufferCharacter(HyphenMinus);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedDashDash);
          }
          if (character == LessThanSign)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedLessThanSign);
          }
          if (character == GreaterThanSign)
          {
            BufferCharacter(GreaterThanSign);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptData);
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            BufferCharacter(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscaped);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInScriptHTMLCommentLikeText);
            RECONSUME_IN(Data);
          }

          BufferCharacter(character);
          ADVANCE_TO(ScriptDataEscaped);
        END_STATE()

        BEGIN_STATE(ScriptDataEscapedLessThanSign)
          if (character == Solidus)
          {
            _temporaryBuffer.clear();
            assert(_bufferedEndTagName.empty());
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedEndTagOpen);
          }
          if (Krys::Text::IsASCIIAlpha(character))
          {
            _temporaryBuffer.clear();
            BufferCharacters(Array {LessThanSign, character});
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapeStart);
          }

          BufferCharacter(LessThanSign);
          RECONSUME_IN(ScriptDataEscaped);
        END_STATE()

        BEGIN_STATE(ScriptDataEscapedEndTagOpen)
          if (Krys::Text::IsASCIIAlpha(character))
          {
            _temporaryBuffer.push_back(character);
            AppendToPossibleEndTag(Krys::Text::ToASCIILower(character));
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedEndTagName);
          }

          BufferCharacters(Array {LessThanSign, Solidus});
          RECONSUME_IN(ScriptDataEscaped);
        END_STATE()

        BEGIN_STATE(ScriptDataEscapedEndTagName)
          if (IsTokenizerWhitespace(character))
          {
            if (IsAppropriateEndTag())
            {
              if (CommitToPartialEndTag(character, TokenizerState::BeforeAttributeName))
              {
                return true;
              }
            }
            SWITCH_TO(BeforeAttributeName);
          }
          if (character == Solidus)
          {
            if (IsAppropriateEndTag())
            {
              if (CommitToPartialEndTag(Solidus, TokenizerState::SelfClosingStartTag))
              {
                return true;
              }
            }
            SWITCH_TO(SelfClosingStartTag);
          }
          if (character == GreaterThanSign)
          {
            if (IsAppropriateEndTag())
            {
              return CommitToCompleteEndTag();
            }
          }
          if (Krys::Text::IsASCIIAlpha(character))
          {
            _temporaryBuffer.push_back(character);
            AppendToPossibleEndTag(Krys::Text::ToASCIILower(character));
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataEscapedEndTagName);
          }

          BufferCharacters(Array {LessThanSign, Solidus});
          BufferCharacters(_temporaryBuffer);
          _bufferedEndTagName.clear();
          _temporaryBuffer.clear();
          RECONSUME_IN(ScriptDataEscaped);
        END_STATE()

        BEGIN_STATE(ScriptDataDoubleEscapeStart)
          if (IsTokenizerWhitespace(character) || character == Solidus || character == GreaterThanSign)
          {
            BufferCharacter(character);
            if (IsTemporaryBufferEqualTo(U"script"))
            {
              ADVANCE_TO(ScriptDataDoubleEscaped);
            }
            else
            {
              ADVANCE_TO(ScriptDataEscaped);
            }
          }
          if (Krys::Text::IsASCIIAlpha(character))
          {
            BufferCharacter(character);
            _temporaryBuffer.push_back(Krys::Text::ToASCIILower(character));
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapeStart);
          }

          RECONSUME_IN(ScriptDataEscaped);
        END_STATE()

        BEGIN_STATE(ScriptDataDoubleEscaped)
          if (character == HyphenMinus)
          {
            BufferCharacter(HyphenMinus);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapedDash);
          }
          if (character == LessThanSign)
          {
            BufferCharacter(LessThanSign);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapedLessThanSign);
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            BufferCharacter(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscaped);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInScriptHTMLCommentLikeText);
            RECONSUME_IN(Data);
          }

          BufferCharacter(character);
          ADVANCE_TO(ScriptDataDoubleEscaped);
        END_STATE()

        BEGIN_STATE(ScriptDataDoubleEscapedDash)
          if (character == HyphenMinus)
          {
            BufferCharacter(HyphenMinus);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapedDashDash);
          }
          if (character == LessThanSign)
          {
            BufferCharacter(LessThanSign);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapedLessThanSign);
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            BufferCharacter(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscaped);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInScriptHTMLCommentLikeText);
            RECONSUME_IN(Data);
          }

          BufferCharacter(character);
          ADVANCE_TO(ScriptDataDoubleEscaped);
        END_STATE()

        BEGIN_STATE(ScriptDataDoubleEscapedDashDash)
          if (character == HyphenMinus)
          {
            BufferCharacter(HyphenMinus);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapedDashDash);
          }
          if (character == LessThanSign)
          {
            BufferCharacter(LessThanSign);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapedLessThanSign);
          }
          if (character == GreaterThanSign)
          {
            BufferCharacter(GreaterThanSign);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptData);
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            BufferCharacter(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscaped);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInScriptHTMLCommentLikeText);
            RECONSUME_IN(Data);
          }

          BufferCharacter(character);
          ADVANCE_TO(ScriptDataDoubleEscaped);
        END_STATE()

        BEGIN_STATE(ScriptDataDoubleEscapedLessThanSign)
          if (character == Solidus)
          {
            _temporaryBuffer.clear();
            assert(_bufferedEndTagName.empty());
            BufferCharacter(Solidus);
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapeEnd);
          }

          RECONSUME_IN(ScriptDataDoubleEscaped);
        END_STATE()

        BEGIN_STATE(ScriptDataDoubleEscapeEnd)
          if (IsTokenizerWhitespace(character) || character == Solidus || character == GreaterThanSign)
          {
            BufferCharacter(character);
            if (IsTemporaryBufferEqualTo(U"script"))
            {
              ADVANCE_TO(ScriptDataEscaped);
            }
            else
            {
              ADVANCE_TO(ScriptDataDoubleEscaped);
            }
          }
          if (Krys::Text::IsASCIIAlpha(character))
          {
            BufferCharacter(character);
            _temporaryBuffer.push_back(Krys::Text::ToASCIILower(character));
            ADVANCE_PAST_NON_NEWLINE_TO(ScriptDataDoubleEscapeEnd);
          }

          RECONSUME_IN(ScriptDataDoubleEscaped);
        END_STATE()

        BEGIN_STATE(BeforeAttributeName)
          if (IsTokenizerWhitespace(character))
          {
            ADVANCE_TO(BeforeAttributeName);
          }
          if (character == Solidus || character == GreaterThanSign)
          {
            RECONSUME_IN(AfterAttributeName);
          }
          if (character == EqualSign)
          {
            ParserError(HTMLParseError::UnexpectedEqualsSignBeforeAttributeName);
            _token.BeginAttribute();
            _token.AppendToCurrentAttributeName(EqualSign);
            ADVANCE_PAST_NON_NEWLINE_TO(AttributeName);
          }

          _token.BeginAttribute();
          RECONSUME_IN(AttributeName);
        END_STATE()

        BEGIN_STATE(AttributeName)
          if (IsTokenizerWhitespace(character) || character == Solidus || character == GreaterThanSign
              || character == EndOfFile)
          {
            RECONSUME_IN(AfterAttributeName);
          }
          if (character == EqualSign)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(BeforeAttributeValue);
          }
          if (Krys::Text::IsASCIIUpper(character))
          {
            _token.AppendToCurrentAttributeName(Krys::Text::ToASCIILowerUnchecked(character));
            ADVANCE_PAST_NON_NEWLINE_TO(AttributeName);
          }
          if (character == Null)
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            _token.AppendToCurrentAttributeName(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(AttributeName);
          }
          if (character == QuotationMark || character == Apostrophe || character == LessThanSign)
          {
            ParserError(HTMLParseError::UnexpectedCharacterInAttributeName);
            // Treat it as per 'anything else' case below.
          }

          _token.AppendToCurrentAttributeName(character);
          ADVANCE_PAST_NON_NEWLINE_TO(AttributeName);
        END_STATE()

        BEGIN_STATE(AfterAttributeName)
          if (IsTokenizerWhitespace(character))
          {
            // Ignore the character.
            ADVANCE_TO(AfterAttributeName);
          }
          if (character == Solidus)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(SelfClosingStartTag);
          }
          if (character == EqualSign)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(BeforeAttributeValue);
          }
          if (character == GreaterThanSign)
          {
            return EmitTagToken();
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInTag);
            RECONSUME_IN(Data);
          }

          EndAttribute();
          BeginAttribute();
          RECONSUME_IN(AttributeName);
        END_STATE()

        BEGIN_STATE(BeforeAttributeValue)
          if (IsTokenizerWhitespace(character))
          {
            // Ignore the character.
            ADVANCE_TO(BeforeAttributeValue);
          }
          if (character == QuotationMark)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(AttributeValueDoubleQuoted);
          }
          if (character == Apostrophe)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(AttributeValueSingleQuoted);
          }
          if (character == GreaterThanSign)
          {
            ParserError(HTMLParseError::MissingAttributeValue);
            EndAttribute();
            return EmitTagToken();
          }

          RECONSUME_IN(AttributeValueUnquoted);
        END_STATE()

        BEGIN_STATE(AttributeValueDoubleQuoted)
          if (character == QuotationMark)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(AfterAttributeValueQuoted);
          }
          if (character == Ampersand)
          {
            _characterReferenceReturnState = TokenizerState::AttributeValueDoubleQuoted;
            ADVANCE_PAST_NON_NEWLINE_TO(CharacterReference);
          }
          if (character == Null)
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            _token.AppendToCurrentAttributeValue(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(AttributeValueDoubleQuoted);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInTag);
            EndAttribute();
            RECONSUME_IN(Data);
          }

          _token.AppendToCurrentAttributeValue(character);
          ADVANCE_TO(AttributeValueDoubleQuoted);
        END_STATE()

        BEGIN_STATE(AttributeValueSingleQuoted)
          if (character == Apostrophe)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(AfterAttributeValueQuoted);
          }
          if (character == Ampersand)
          {
            _characterReferenceReturnState = TokenizerState::AttributeValueSingleQuoted;
            ADVANCE_PAST_NON_NEWLINE_TO(CharacterReference);
          }
          if (character == Null)
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            _token.AppendToCurrentAttributeValue(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(AttributeValueSingleQuoted);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInTag);
            EndAttribute();
            RECONSUME_IN(Data);
          }

          _token.AppendToCurrentAttributeValue(character);
          ADVANCE_TO(AttributeValueSingleQuoted);
        END_STATE()

        BEGIN_STATE(AttributeValueUnquoted)
          if (IsTokenizerWhitespace(character))
          {
            EndAttribute();
            ADVANCE_TO(BeforeAttributeName);
          }
          if (character == Ampersand)
          {
            _characterReferenceReturnState = TokenizerState::AttributeValueUnquoted;
            ADVANCE_PAST_NON_NEWLINE_TO(CharacterReference);
          }
          if (character == GreaterThanSign)
          {
            EndAttribute();
            return EmitTagToken();
          }
          if (character == Null)
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            _token.AppendToCurrentAttributeValue(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(AttributeValueUnquoted);
          }
          if (character == QuotationMark || character == Apostrophe || character == LessThanSign
              || character == EqualSign || character == GraveAccent)
          {
            ParserError(HTMLParseError::UnexpectedCharacterInUnquotedAttributeValue);
            _token.AppendToCurrentAttributeValue(character);
            ADVANCE_PAST_NON_NEWLINE_TO(AttributeValueUnquoted);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInTag);
            EndAttribute();
            RECONSUME_IN(Data);
          }

          _token.AppendToCurrentAttributeValue(character);
          ADVANCE_PAST_NON_NEWLINE_TO(AttributeValueUnquoted);
        END_STATE()

        BEGIN_STATE(AfterAttributeValueQuoted)
          if (IsTokenizerWhitespace(character))
          {
            EndAttribute();
            ADVANCE_TO(BeforeAttributeName);
          }
          if (character == Solidus)
          {
            EndAttribute();
            ADVANCE_PAST_NON_NEWLINE_TO(SelfClosingStartTag);
          }
          if (character == GreaterThanSign)
          {
            EndAttribute();
            return EmitTagToken();
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInTag);
            EndAttribute();
            RECONSUME_IN(Data);
          }

          ParserError(HTMLParseError::MissingWhitespaceBetweenAttributes);
          EndAttribute();
          RECONSUME_IN(BeforeAttributeName);
        END_STATE()

        BEGIN_STATE(SelfClosingStartTag)
          if (character == GreaterThanSign)
          {
            _token.SetSelfClosingFlag();
            return EmitTagToken();
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInTag);
            RECONSUME_IN(Data);
          }

          ParserError(HTMLParseError::UnexpectedSolidusInTag);
          RECONSUME_IN(BeforeAttributeName);
        END_STATE()

        BEGIN_STATE(BogusComment)
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
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            _token.AppendToComment(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(BogusComment);
          }

          _token.AppendToComment(character);
          ADVANCE_PAST_NON_NEWLINE_TO(BogusComment);
        END_STATE()

        BEGIN_STATE(MarkupDeclarationOpen)
          using Krys::Text::operator""_s;

          if (character == HyphenMinus)
          {
            auto result = _input.AdvancePast<false>("--"_s);
            if (result == HTMLInputStream::MatchResult::Matched)
            {
              _token.BeginComment();
              SWITCH_TO(CommentStart);
            }
            if (result == HTMLInputStream::MatchResult::NotEnoughCharacters)
            {
              RETURN_IN_CURRENT_STATE(HasBufferedCharacterToken());
            }
          }
          else if (Krys::Text::MatchesASCIINormalizedLiteral(character, 'd'))
          {
            auto result = _input.AdvancePast("doctype"_s);
            if (result == HTMLInputStream::MatchResult::Matched)
            {
              SWITCH_TO(DOCTYPE);
            }
            if (result == HTMLInputStream::MatchResult::NotEnoughCharacters)
            {
              RETURN_IN_CURRENT_STATE(HasBufferedCharacterToken());
            }
          }
          else if (character == '[')
          {
            auto result = _input.AdvancePast<false>("[CDATA[");
            if (result == HTMLInputStream::MatchResult::Matched)
            {
              if (IsCDATAAllowed())
              {
                SWITCH_TO(CDATASection);
              }
              else
              {
                ParserError(HTMLParseError::CDATAInHTMLContent);
                _token.BeginComment();
                _token.AppendToComment("[CDATA["_s);
                SWITCH_TO(BogusComment);
              }
            }
            if (result == HTMLInputStream::MatchResult::NotEnoughCharacters)
            {
              RETURN_IN_CURRENT_STATE(HasBufferedCharacterToken());
            }
          }

          ParserError(HTMLParseError::IncorrectlyOpenedComment);
          _token.BeginComment();
          RECONSUME_IN(BogusComment);
        END_STATE()

        BEGIN_STATE(CommentStart)
          if (character == HyphenMinus)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(CommentStartDash);
          }
          if (character == GreaterThanSign)
          {
            ParserError(HTMLParseError::AbruptClosingOfEmptyComment);
            return EmitCommentToken(true);
          }

          RECONSUME_IN(Comment);
        END_STATE()

        BEGIN_STATE(CommentStartDash)
          if (character == HyphenMinus)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(CommentEnd);
          }
          if (character == GreaterThanSign)
          {
            ParserError(HTMLParseError::AbruptClosingOfEmptyComment);
            return EmitCommentToken(true);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInComment);
            return EmitCommentToken(false);
          }

          _token.AppendToComment(HyphenMinus);
          RECONSUME_IN(Comment);
        END_STATE()

        BEGIN_STATE(Comment)
          if (character == LessThanSign)
          {
            _token.AppendToComment(LessThanSign);
            ADVANCE_PAST_NON_NEWLINE_TO(CommentLessThanSign);
          }
          if (character == HyphenMinus)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(CommentEndDash);
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            _token.AppendToComment(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(Comment);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInComment);
            return EmitCommentToken(false);
          }

          _token.AppendToComment(character);
          ADVANCE_TO(Comment);
        END_STATE()

        BEGIN_STATE(CommentLessThanSign)
          if (character == ExclamationMark)
          {
            _token.AppendToComment(ExclamationMark);
            ADVANCE_PAST_NON_NEWLINE_TO(CommentLessThanSignBang);
          }
          if (character == LessThanSign)
          {
            _token.AppendToComment(LessThanSign);
            ADVANCE_PAST_NON_NEWLINE_TO(CommentLessThanSign);
          }

          RECONSUME_IN(Comment);
        END_STATE()

        BEGIN_STATE(CommentLessThanSignBang)
          if (character == HyphenMinus)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(CommentLessThanSignBangDash);
          }

          RECONSUME_IN(Comment);
        END_STATE()

        BEGIN_STATE(CommentLessThanSignBangDash)
          if (character == HyphenMinus)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(CommentLessThanSignBangDashDash);
          }

          RECONSUME_IN(CommentEndDash);
        END_STATE()

        BEGIN_STATE(CommentLessThanSignBangDashDash)
          if (character == GreaterThanSign || character == EndOfFile)
          {
            RECONSUME_IN(CommentEnd);
          }

          ParserError(HTMLParseError::NestedComment);
          RECONSUME_IN(CommentEnd);
        END_STATE()

        BEGIN_STATE(CommentEndDash)
          if (character == HyphenMinus)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(CommentEnd);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInComment);
            return EmitCommentToken(false);
          }

          _token.AppendToComment(HyphenMinus);
          RECONSUME_IN(Comment);
        END_STATE()

        BEGIN_STATE(CommentEnd)
          using Krys::Text::operator""_s;

          if (character == GreaterThanSign)
          {
            return EmitCommentToken(true);
          }
          if (character == ExclamationMark)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(CommentEndBang);
          }
          if (character == HyphenMinus)
          {
            _token.AppendToComment(HyphenMinus);
            ADVANCE_PAST_NON_NEWLINE_TO(CommentEnd);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInComment);
            return EmitCommentToken(false);
          }

          _token.AppendToComment("--"_s);
          RECONSUME_IN(Comment);
        END_STATE()

        BEGIN_STATE(CommentEndBang)
          using Krys::Text::operator""_s;

          if (character == HyphenMinus)
          {
            _token.AppendToComment("--!"_s);
            ADVANCE_PAST_NON_NEWLINE_TO(CommentEndDash);
          }
          if (character == GreaterThanSign)
          {
            ParserError(HTMLParseError::IncorrectlyClosedComment);
            return EmitCommentToken(true);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInComment);
            return EmitCommentToken(false);
          }

          _token.AppendToComment("--!"_s);
          RECONSUME_IN(Comment);
        END_STATE()

        BEGIN_STATE(DOCTYPE)
          if (IsTokenizerWhitespace(character))
          {
            ADVANCE_TO(BeforeDOCTYPEName);
          }
          if (character == GreaterThanSign)
          {
            RECONSUME_IN(BeforeDOCTYPEName);
          }
          if (character == EndOfFile)
          {
            ParserError(HTMLParseError::EOFInDOCTYPE);
            _token.BeginDOCTYPE();
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(false);
          }

          ParserError(HTMLParseError::MissingWhitespaceBeforeDOCTYPEName);
          RECONSUME_IN(BeforeDOCTYPEName);
        END_STATE()

        BEGIN_STATE(BeforeDOCTYPEName)
          if (IsTokenizerWhitespace(character))
          {
            ADVANCE_TO(BeforeDOCTYPEName);
          }
          if (Krys::Text::IsASCIIUpper(character))
          {
            _token.BeginDOCTYPE();
            _token.AppendToName(Krys::Text::ToASCIILowerUnchecked(character));
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEName);
          }
          if (character == Null)
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            _token.BeginDOCTYPE();
            _token.AppendToName(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEName);
          }
          if (character == GreaterThanSign)
          {
            ParserError(HTMLParseError::MissingDOCTYPEName);
            _token.BeginDOCTYPE();
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(true);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInDOCTYPE);
            _token.BeginDOCTYPE();
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(false);
          }

          _token.BeginDOCTYPE();
          _token.AppendToName(character);
          ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEName);
        END_STATE()

        BEGIN_STATE(DOCTYPEName)
          if (IsTokenizerWhitespace(character))
          {
            ADVANCE_TO(AfterDOCTYPEName);
          }
          if (character == GreaterThanSign)
          {
            return EmitDOCTYPEToken(true);
          }
          if (Krys::Text::IsASCIIUpper(character))
          {
            _token.AppendToName(Krys::Text::ToASCIILowerUnchecked(character));
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEName);
          }
          if (character == Null)
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            _token.AppendToName(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEName);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInDOCTYPE);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(false);
          }

          _token.AppendToName(character);
          ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEName);
        END_STATE()

        BEGIN_STATE(AfterDOCTYPEName)
          using Krys::Text::operator""_s;

          if (IsTokenizerWhitespace(character))
          {
            ADVANCE_TO(AfterDOCTYPEName);
          }
          if (character == GreaterThanSign)
          {
            return EmitDOCTYPEToken(true);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInDOCTYPE);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(false);
          }
          if (Krys::Text::MatchesASCIINormalizedLiteral(character, 'p'))
          {
            auto result = _input.AdvancePast("public"_s);
            if (result == HTMLInputStream::MatchResult::Matched)
            {
              SWITCH_TO(AfterDOCTYPEPublicKeyword);
            }
            if (result == HTMLInputStream::MatchResult::NotEnoughCharacters)
            {
              RETURN_IN_CURRENT_STATE(HasBufferedCharacterToken());
            }
          }
          if (Krys::Text::MatchesASCIINormalizedLiteral(character, 's'))
          {
            auto result = _input.AdvancePast("system"_s);
            if (result == HTMLInputStream::MatchResult::Matched)
            {
              SWITCH_TO(AfterDOCTYPESystemKeyword);
            }
            if (result == HTMLInputStream::MatchResult::NotEnoughCharacters)
            {
              RETURN_IN_CURRENT_STATE(HasBufferedCharacterToken());
            }
          }

          ParserError(HTMLParseError::InvalidCharacterSequenceAfterDOCTYPEName);
          _token.SetDOCTYPEForceQuirks();
          RECONSUME_IN(BogusDOCTYPE);
        END_STATE()

        BEGIN_STATE(AfterDOCTYPEPublicKeyword)
          if (IsTokenizerWhitespace(character))
          {
            ADVANCE_TO(BeforeDOCTYPEPublicIdentifier);
          }
          if (character == QuotationMark)
          {
            ParserError(HTMLParseError::MissingWhitespaceAfterDOCTYPEPublicKeyword);
            _token.SetPublicIdentifierToEmptyString();
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEPublicIdentifierDoubleQuoted);
          }
          if (character == Apostrophe)
          {
            ParserError(HTMLParseError::MissingWhitespaceAfterDOCTYPEPublicKeyword);
            _token.SetPublicIdentifierToEmptyString();
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEPublicIdentifierSingleQuoted);
          }
          if (character == GreaterThanSign)
          {
            ParserError(HTMLParseError::MissingDOCTYPEPublicIdentifier);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(true);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInDOCTYPE);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(false);
          }

          ParserError(HTMLParseError::MissingQuoteBeforeDOCTYPEPublicIdentifier);
          _token.SetDOCTYPEForceQuirks();
          RECONSUME_IN(BogusDOCTYPE);
        END_STATE()

        BEGIN_STATE(BeforeDOCTYPEPublicIdentifier)
          if (IsTokenizerWhitespace(character))
          {
            ADVANCE_TO(BeforeDOCTYPEPublicIdentifier);
          }
          if (character == QuotationMark)
          {
            _token.SetPublicIdentifierToEmptyString();
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEPublicIdentifierDoubleQuoted);
          }
          if (character == Apostrophe)
          {
            _token.SetPublicIdentifierToEmptyString();
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEPublicIdentifierSingleQuoted);
          }
          if (character == GreaterThanSign)
          {
            ParserError(HTMLParseError::MissingDOCTYPEPublicIdentifier);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(true);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInDOCTYPE);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(false);
          }

          ParserError(HTMLParseError::MissingQuoteBeforeDOCTYPEPublicIdentifier);
          _token.SetDOCTYPEForceQuirks();
          RECONSUME_IN(BogusDOCTYPE);
        END_STATE()

        BEGIN_STATE(DOCTYPEPublicIdentifierDoubleQuoted)
          if (character == QuotationMark)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(AfterDOCTYPEPublicIdentifier);
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            _token.AppendToPublicIdentifier(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEPublicIdentifierDoubleQuoted);
          }
          if (character == GreaterThanSign)
          {
            ParserError(HTMLParseError::AbruptDOCTYPEPublicIdentifier);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(true);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInDOCTYPE);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(false);
          }

          _token.AppendToPublicIdentifier(character);
          ADVANCE_TO(DOCTYPEPublicIdentifierDoubleQuoted);
        END_STATE()

        BEGIN_STATE(DOCTYPEPublicIdentifierSingleQuoted)
          if (character == Apostrophe)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(AfterDOCTYPEPublicIdentifier);
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            _token.AppendToPublicIdentifier(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPEPublicIdentifierSingleQuoted);
          }
          if (character == GreaterThanSign)
          {
            ParserError(HTMLParseError::AbruptDOCTYPEPublicIdentifier);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(true);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInDOCTYPE);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(false);
          }

          _token.AppendToPublicIdentifier(character);
          ADVANCE_TO(DOCTYPEPublicIdentifierSingleQuoted);
        END_STATE()

        BEGIN_STATE(AfterDOCTYPEPublicIdentifier)
          if (IsTokenizerWhitespace(character))
          {
            ADVANCE_TO(BetweenDOCTYPEPublicAndSystemIdentifiers);
          }
          if (character == GreaterThanSign)
          {
            return EmitDOCTYPEToken(true);
          }
          if (character == QuotationMark)
          {
            ParserError(HTMLParseError::MissingWhitespaceBetweenDOCTYPEPublicAndSystemIdentifiers);
            _token.SetSystemIdentifierToEmptyString();
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierDoubleQuoted);
          }
          if (character == Apostrophe)
          {
            ParserError(HTMLParseError::MissingWhitespaceBetweenDOCTYPEPublicAndSystemIdentifiers);
            _token.SetSystemIdentifierToEmptyString();
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierSingleQuoted);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInDOCTYPE);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(false);
          }

          ParserError(HTMLParseError::MissingQuoteBeforeDOCTYPESystemIdentifier);
          _token.SetDOCTYPEForceQuirks();
          RECONSUME_IN(BogusDOCTYPE);
        END_STATE()

        BEGIN_STATE(BetweenDOCTYPEPublicAndSystemIdentifiers)
          if (IsTokenizerWhitespace(character))
          {
            ADVANCE_TO(BetweenDOCTYPEPublicAndSystemIdentifiers);
          }
          if (character == GreaterThanSign)
          {
            return EmitDOCTYPEToken(true);
          }
          if (character == QuotationMark)
          {
            _token.SetSystemIdentifierToEmptyString();
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierDoubleQuoted);
          }
          if (character == Apostrophe)
          {
            _token.SetSystemIdentifierToEmptyString();
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierSingleQuoted);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInDOCTYPE);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(false);
          }

          ParserError(HTMLParseError::MissingQuoteBeforeDOCTYPESystemIdentifier);
          _token.SetDOCTYPEForceQuirks();
          RECONSUME_IN(BogusDOCTYPE);
        END_STATE()

        BEGIN_STATE(AfterDOCTYPESystemKeyword)
          if (IsTokenizerWhitespace(character))
          {
            ADVANCE_TO(BeforeDOCTYPESystemIdentifier);
          }
          if (character == QuotationMark)
          {
            ParserError(HTMLParseError::MissingWhitespaceAfterDOCTYPESystemKeyword);
            _token.SetSystemIdentifierToEmptyString();
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierDoubleQuoted);
          }
          if (character == Apostrophe)
          {
            ParserError(HTMLParseError::MissingWhitespaceAfterDOCTYPESystemKeyword);
            _token.SetSystemIdentifierToEmptyString();
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierSingleQuoted);
          }
          if (character == GreaterThanSign)
          {
            ParserError(HTMLParseError::MissingDOCTYPESystemIdentifier);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(true);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInDOCTYPE);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(false);
          }

          ParserError(HTMLParseError::MissingQuoteBeforeDOCTYPESystemIdentifier);
          _token.SetDOCTYPEForceQuirks();
          RECONSUME_IN(BogusDOCTYPE);
        END_STATE()

        BEGIN_STATE(BeforeDOCTYPESystemIdentifier)
          if (IsTokenizerWhitespace(character))
          {
            ADVANCE_TO(BeforeDOCTYPESystemIdentifier);
          }
          if (character == QuotationMark)
          {
            _token.SetSystemIdentifierToEmptyString();
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierDoubleQuoted);
          }
          if (character == Apostrophe)
          {
            _token.SetSystemIdentifierToEmptyString();
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierSingleQuoted);
          }
          if (character == GreaterThanSign)
          {
            ParserError(HTMLParseError::MissingDOCTYPESystemIdentifier);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(true);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInDOCTYPE);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(false);
          }

          ParserError(HTMLParseError::MissingQuoteBeforeDOCTYPESystemIdentifier);
          _token.SetDOCTYPEForceQuirks();
          RECONSUME_IN(BogusDOCTYPE);
        END_STATE()

        BEGIN_STATE(DOCTYPESystemIdentifierDoubleQuoted)
          if (character == QuotationMark)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(AfterDOCTYPESystemIdentifier);
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            _token.AppendToSystemIdentifier(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierDoubleQuoted);
          }
          if (character == GreaterThanSign)
          {
            ParserError(HTMLParseError::AbruptDOCTYPESystemIdentifier);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(true);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInDOCTYPE);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(false);
          }

          _token.AppendToSystemIdentifier(character);
          ADVANCE_TO(DOCTYPESystemIdentifierDoubleQuoted);
        END_STATE()

        BEGIN_STATE(DOCTYPESystemIdentifierSingleQuoted)
          if (character == Apostrophe)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(AfterDOCTYPESystemIdentifier);
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            _token.AppendToSystemIdentifier(Replacement);
            ADVANCE_PAST_NON_NEWLINE_TO(DOCTYPESystemIdentifierSingleQuoted);
          }
          if (character == GreaterThanSign)
          {
            ParserError(HTMLParseError::AbruptDOCTYPESystemIdentifier);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(true);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInDOCTYPE);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(false);
          }

          _token.AppendToSystemIdentifier(character);
          ADVANCE_TO(DOCTYPESystemIdentifierSingleQuoted);
        END_STATE()

        BEGIN_STATE(AfterDOCTYPESystemIdentifier)
          if (IsTokenizerWhitespace(character))
          {
            ADVANCE_TO(AfterDOCTYPESystemIdentifier);
          }
          if (character == GreaterThanSign)
          {
            return EmitDOCTYPEToken(true);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInDOCTYPE);
            _token.SetDOCTYPEForceQuirks();
            return EmitDOCTYPEToken(false);
          }

          ParserError(HTMLParseError::UnexpectedCharacterAfterDOCTYPESystemIdentifier);
          RECONSUME_IN(BogusDOCTYPE);
        END_STATE()

        BEGIN_STATE(BogusDOCTYPE)
          if (character == GreaterThanSign)
          {
            return EmitDOCTYPEToken(true);
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            ADVANCE_PAST_NON_NEWLINE_TO(BogusDOCTYPE);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            return EmitDOCTYPEToken(false);
          }

          ADVANCE_TO(BogusDOCTYPE);
        END_STATE()

        BEGIN_STATE(CDATASection)
          if (character == RightSquareBracket)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(CDATASectionBracket);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFInCDATA);
            RECONSUME_IN(Data);
          }

          _token.AppendToCharacters(character);
          ADVANCE_TO(CDATASection);
        END_STATE()

        BEGIN_STATE(CDATASectionBracket)
          if (character == RightSquareBracket)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(CDATASectionEnd);
          }

          _token.AppendToCharacters(RightSquareBracket);
          RECONSUME_IN(CDATASection);
        END_STATE()

        BEGIN_STATE(CDATASectionEnd)
          if (character == RightSquareBracket)
          {
            _token.AppendToCharacters(RightSquareBracket);
            ADVANCE_PAST_NON_NEWLINE_TO(CDATASectionEnd);
          }
          if (character == GreaterThanSign)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(Data);
          }

          _token.AppendToCharacters(RightSquareBracket);
          _token.AppendToCharacters(RightSquareBracket);
          RECONSUME_IN(CDATASection);
        END_STATE()

        BEGIN_STATE(CharacterReference)
          _temporaryBuffer.clear();
          _temporaryBuffer.push_back(Ampersand);

          if (Krys::Text::IsASCIIAlphanumeric(character))
          {
            _temporaryBuffer.push_back(character);
            _namedCharacterReferenceMatchEntries = SearchNamedCharacterReferences(_temporaryBuffer);
            _longestCharacterReferenceMatch = nullptr;
            ADVANCE_PAST_NON_NEWLINE_TO(NamedCharacterReference);
          }
          if (character == NumberSign)
          {
            _temporaryBuffer.push_back(character);
            ADVANCE_PAST_NON_NEWLINE_TO(NumericCharacterReference);
          }

          FlushCodePointsConsumedAsACharacterReference();
          RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE();
        END_STATE()

        BEGIN_STATE(NamedCharacterReference)
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
              ADVANCE_TO_CHARACTER_REFERENCE_RETURN_STATE();
            }

            // Keep track of the longest match so far
            if (first.Name.size() == _temporaryBuffer.size())
            {
              _longestCharacterReferenceMatch = &first;
            }

            ADVANCE_PAST_NON_NEWLINE_TO(NamedCharacterReference);
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
                && (character == EqualSign || Krys::Text::IsASCIIAlphanumeric(character)))
            {
              AppendToTemporaryBuffer(match.Name);
              AppendToTemporaryBuffer(nonMatchingCharacters);
              FlushCodePointsConsumedAsACharacterReference();
              RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE();
            }

            AppendToTemporaryBuffer(match.ToSpan());
            AppendToTemporaryBuffer(nonMatchingCharacters);
            FlushCodePointsConsumedAsACharacterReference();

            if (match.Name.back() != Semicolon)
            {
              ParserError(HTMLParseError::MissingSemicolonAfterCharacterReference);
            }

            RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE();
          }

          // no matches at all
          _temporaryBuffer.pop_back(); // remove the last character added so we can reconsume it
          FlushCodePointsConsumedAsACharacterReference();
          RECONSUME_IN(AmbiguousAmpersand);
        END_STATE()

        BEGIN_STATE(AmbiguousAmpersand)
          if (Krys::Text::IsASCIIAlphanumeric(character))
          {
            if (CharacterReferenceWasConsumedAsPartOfAnAttribute())
            {
              _token.AppendToCurrentAttributeValue(character);
            }
            else
            {
              _token.AppendToCharacters(character);
            }
            ADVANCE_TO(AmbiguousAmpersand);
          }
          if (character == Semicolon)
          {
            ParserError(HTMLParseError::UnknownNamedCharacterReference);
            RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE();
          }

          RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE();
        END_STATE()

        BEGIN_STATE(NumericCharacterReference)
          _characterReferenceCode = 0;
          if (Krys::Text::MatchesASCIINormalizedLiteral(character, 'x'))
          {
            _temporaryBuffer.push_back(character);
            ADVANCE_PAST_NON_NEWLINE_TO(HexadecimalCharacterReferenceStart);
          }

          RECONSUME_IN(DecimalCharacterReferenceStart);
        END_STATE()

        BEGIN_STATE(HexadecimalCharacterReferenceStart)
          if (Krys::Text::IsASCIIHexDigit(character))
          {
            RECONSUME_IN(HexadecimalCharacterReference);
          }

          ParserError(HTMLParseError::AbsenceOfDigitsInNumericCharacterReference);
          FlushCodePointsConsumedAsACharacterReference();
          RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE();
        END_STATE()

        BEGIN_STATE(DecimalCharacterReferenceStart)
          if (Krys::Text::IsASCIIDigit(character))
          {
            RECONSUME_IN(DecimalCharacterReference);
          }

          ParserError(HTMLParseError::AbsenceOfDigitsInNumericCharacterReference);
          FlushCodePointsConsumedAsACharacterReference();
          RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE();
        END_STATE()

        BEGIN_STATE(HexadecimalCharacterReference)
          if (Krys::Text::IsASCIIDigit(character))
          {
            _characterReferenceCode *= 16;
            _characterReferenceCode += static_cast<int64>(character - '0');
            ADVANCE_PAST_NON_NEWLINE_TO(HexadecimalCharacterReference);
          }
          if (Krys::Text::IsASCIIHexDigit(character))
          {
            _characterReferenceCode *= 16;
            _characterReferenceCode += static_cast<int64>(Krys::Text::ToASCIIHexValue(character));
            ADVANCE_PAST_NON_NEWLINE_TO(HexadecimalCharacterReference);
          }
          if (character != Semicolon)
          {
            ParserError(HTMLParseError::MissingSemicolonAfterCharacterReference);
          }

          RECONSUME_IN(NumericCharacterReferenceEnd);
        END_STATE()

        BEGIN_STATE(DecimalCharacterReference)
          if (Krys::Text::IsASCIIDigit(character))
          {
            _characterReferenceCode *= 10;
            _characterReferenceCode += static_cast<int64>(character - '0');
            ADVANCE_PAST_NON_NEWLINE_TO(DecimalCharacterReference);
          }
          if (character != Semicolon)
          {
            ParserError(HTMLParseError::MissingSemicolonAfterCharacterReference);
          }

          RECONSUME_IN(NumericCharacterReferenceEnd);
        END_STATE()

        BEGIN_STATE(NumericCharacterReferenceEnd)
          if (_characterReferenceCode == Null)
          {
            ParserError(HTMLParseError::NullCharacterReference);
            _characterReferenceCode = Replacement;
          }
          else if (_characterReferenceCode > 0x10FFFF)
          {
            ParserError(HTMLParseError::CharacterReferenceOutsideUnicodeRange);
            _characterReferenceCode = Replacement;
          }
          else if (Krys::Text::Unicode::IsSurrogate(static_cast<char32>(_characterReferenceCode)))
          {
            ParserError(HTMLParseError::SurrogateCharacterReference);
            _characterReferenceCode = Replacement;
          }
          else if (Krys::Text::Unicode::IsNonCharacter(static_cast<char32>(_characterReferenceCode)))
          {
            ParserError(HTMLParseError::NonCharacterCharacterReference);
          }
          else if (_characterReferenceCode == '\x0D'
                   || Krys::Text::IsASCIINonWhitespaceControl(static_cast<char32>(_characterReferenceCode)))
          {
            ParserError(HTMLParseError::ControlCharacterReference);
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
            ADVANCE_TO_CHARACTER_REFERENCE_RETURN_STATE();
          }

          RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE();
        END_STATE()
      }

      std::unreachable();
      return false;
    }

#undef BEGIN_STATE
#undef END_STATE
#undef RETURN_IN_CURRENT_STATE
#undef RECONSUME_IN
#undef ADVANCE_TO
#undef ADVANCE_PAST_NON_NEWLINE_TO
#undef RECONSUME_IN_CHARACTER_REFERENCE_RETURN_STATE
#undef ADVANCE_TO_CHARACTER_REFERENCE_RETURN_STATE
  };
}