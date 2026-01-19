#pragma once

#include "Krystal.HTML/Parser/HTMLInputStream.hpp"
#include "Krystal.HTML/Parser/HTMLParseError.hpp"
#include "Krystal.HTML/Parser/HTMLToken.hpp"
#include "Krystal.HTML/Parser/TokenizerState.hpp"
#include "Krystal.HTML/StronglyTypedValues.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Utils/ReferenceWrapper.hpp"
#include "Krystal.Text/ASCII.hpp"
#include "Krystal.Text/Unicode.hpp"

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

    NextTokenPtr(NextTokenPtr &&other) noexcept : _token(other._token)
    {
      other._token = nullptr;
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
  private:
    TokenizerState _state {TokenizerState::Data};
    HTMLToken _token;
    ReferenceWrapper<HTMLInputStream> _input;
    utf32_string _appropriateEndTagName;
    utf32_string _bufferedEndTagName;

    /// @see https://html.spec.whatwg.org/#temporary-buffer
    List<char8> _temporaryBuffer;

  public:
    HTMLTokenizer(HTMLInputStream &input) noexcept : _input(input)
    {
    }

    NextTokenPtr NextToken() noexcept
    {
      return NextTokenPtr(ProcessToken() ? &_token : nullptr);
    }

  private:
    KRYS_NODISCARD bool ProcessToken() noexcept
    {
      if (!_input.get().Peek())
      {
        return HasBufferedCharacterToken();
      }

      return StepTokenizationStateMachine();
    }

    KRYS_NODISCARD bool HasBufferedCharacterToken() const noexcept
    {
      return _token.GetType() == HTMLToken::Type::Character;
    }

    void BufferCharacter(char32 character) noexcept
    {
      assert(character != HTMLInputStream::EOFMarker);
      _token.AddCharacter(character);
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
      _token.AddCharacters(characters);
    }

    void AppendToTemporaryBuffer(char32 character) noexcept
    {
      assert(Text::IsASCII(character));
      _temporaryBuffer.push_back(character);
    }

    KRYS_NODISCARD bool TryConsumeCharacterReference() noexcept
    {
      return false;
    }

    void ParserError(HTMLParseError) noexcept
    {
    }

    bool IsTokenizerWhitespace(char32 character)
    {
      return character == ' ' || character == '\x0A' || character == '\x09' || character == '\x0C';
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
    if (!_input.get().Advance())                                                                             \
    {                                                                                                        \
      _state = TokenizerState::newState;                                                                     \
      return HasBufferedCharacterToken();                                                                    \
    }                                                                                                        \
    character = _input.get().NextInputCharacter();                                                           \
    goto newState;                                                                                           \
  } while (false)

// For when the HTML spec says "consume the next input character ... and switch to the <mumble> state."
#define ADVANCE_PAST_NON_NEWLINE_TO(newState)                                                                \
  do                                                                                                         \
  {                                                                                                          \
    if (!_input.get().Advance())                                                                             \
    {                                                                                                        \
      _state = TokenizerState::newState;                                                                     \
      return HasBufferedCharacterToken();                                                                    \
    }                                                                                                        \
    character = _input.get().NextInputCharacter();                                                           \
    goto newState;                                                                                           \
  } while (false)

// For more complex cases, caller consumes the characters first and then uses this macro.
#define SWITCH_TO(newState)                                                                                  \
  do                                                                                                         \
  {                                                                                                          \
    if (!_input.get().Peek())                                                                                \
    {                                                                                                        \
      _state = TokenizerState::newState;                                                                     \
      return HasBufferedCharacterToken();                                                                    \
    }                                                                                                        \
    character = _input.get().NextInputCharacter();                                                           \
    goto newState;                                                                                           \
  } while (false)

#pragma endregion

    /// @see https://html.spec.whatwg.org/#tokenization
    KRYS_NODISCARD bool StepTokenizationStateMachine() noexcept
    {
      constexpr char32 Null = U'\0';
      constexpr char32 Ampersand = U'&';
      constexpr char32 LessThanSign = U'<';
      constexpr char32 GreaterThanSign = U'>';
      constexpr char32 Solidus = U'/';
      constexpr char32 QuestionMark = U'?';
      constexpr char32 ExclamationMark = U'!';
      constexpr char32 EndOfFile = HTMLInputStream::EOFMarker;
      constexpr char32 Replacement = Text::Unicode::Replacement<char32>;

      char32 character = _input.get().NextInputCharacter();
      switch (_state)
      {
        BEGIN_STATE(Data)
          if (character == Ampersand)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(DataCharacterReference);
          }
          if (character == LessThanSign)
          {
            if (HasBufferedCharacterToken())
            {
              RETURN_IN_CURRENT_STATE(true);
            }
            ADVANCE_PAST_NON_NEWLINE_TO(TagOpen);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            return EmitEOFToken();
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            // It's a parser error but for Data we still emit it anyway as per the spec.
          }

          BufferCharacter(character);
          ADVANCE_TO(Data);
        END_STATE()

        BEGIN_STATE(RCDATA)
          if (character == Ampersand)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(RCDATACharacterReference);
          }
          if (character == LessThanSign)
          {
            ADVANCE_PAST_NON_NEWLINE_TO(RCDATALessThanSign);
          }
          if (character == HTMLInputStream::EOFMarker) KRYS_UNLIKELY
          {
            return EmitEOFToken();
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            character = Replacement;
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
            return EmitEOFToken();
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            character = Replacement;
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
            return EmitEOFToken();
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            character = Replacement;
          }

          BufferCharacter(character);
          ADVANCE_TO(ScriptData);
        END_STATE()

        BEGIN_STATE(PLAINTEXT)
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            return EmitEOFToken();
          }
          if (character == Null) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::UnexpectedNullCharacter);
            character = Replacement;
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
          if (Text::IsASCIIAlpha(character))
          {
            _token.BeginStartTag(Text::ToASCIILower(character));
            ADVANCE_PAST_NON_NEWLINE_TO(TagName);
          }
          if (character == QuestionMark)
          {
            ParserError(HTMLParseError::UnexpectedQuestionMarkInsteadOfTagName);
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
          if (Text::IsASCIIAlpha(character))
          {
            _token.BeginEndTag(Text::ToASCIILower(character));
            ADVANCE_PAST_NON_NEWLINE_TO(TagName);
          }
          if (character == GreaterThanSign)
          {
            ParserError(HTMLParseError::MissingEndTagName);
            // We don't emit the greater than sign, as per the spec.
            ADVANCE_TO(Data);
          }
          if (character == EndOfFile) KRYS_UNLIKELY
          {
            ParserError(HTMLParseError::EOFBeforeTagName);

            BufferCharacters(Array {LessThanSign, Solidus});
            RECONSUME_IN(Data);
          }

          ParserError(HTMLParseError::InvalidFirstCharacterOfTagName);
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
          }
          if (Text::IsASCIIUpper(character))
          {
            _token.AppendToName(Text::ToASCIILowerUnchecked(character));
            ADVANCE_PAST_NON_NEWLINE_TO(TagName);
          }
          if (character == Null)
          {
            _token.AppendToName(Replacement);
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
          if (Text::IsASCIIAlpha(character))
          {
            _token.BeginEndTag(character);
            ADVANCE_PAST_NON_NEWLINE_TO(RCDATAEndTagName);
          }

          BufferCharacters(Array {LessThanSign, Solidus});
          RECONSUME_IN(RCDATA);
        END_STATE()

        BEGIN_STATE(RCDATAEndTagName)
          if (IsTokenizerWhitespace(character))
          {
            // TODO: check end tag token is an appropriate end tag token
            ADVANCE_TO(BeforeAttributeName);
          }
          if (character == Solidus)
          {
            // TODO: check end tag token is an appropriate end tag token
            _temporaryBuffer.clear();
            ADVANCE_PAST_NON_NEWLINE_TO(RCDATAEndTagOpen);
          }
        END_STATE()

        BEGIN_STATE(BogusComment)
          // TODO-STATE
        END_STATE()

        BEGIN_STATE(MarkupDeclarationOpen)
          // TODO-STATE
        END_STATE()

        BEGIN_STATE(DataCharacterReference)
          if (!TryConsumeCharacterReference())
          {
            RETURN_IN_CURRENT_STATE(HasBufferedCharacterToken());
          }
          SWITCH_TO(Data);
        END_STATE()

        BEGIN_STATE(RCDATACharacterReference)
          if (!TryConsumeCharacterReference())
          {
            RETURN_IN_CURRENT_STATE(HasBufferedCharacterToken());
          }
          SWITCH_TO(RCDATA);
        END_STATE()

        BEGIN_STATE(RAWTEXTLessThanSign)
          // TODO-STATE
        END_STATE()

        BEGIN_STATE(RAWTEXTEndTagName)
          // TODO-STATE
        END_STATE()

        BEGIN_STATE(ScriptDataLessThanSign)
          // TODO-STATE
        END_STATE()

        BEGIN_STATE(BeforeAttributeName)
          // TODO-STATE
        END_STATE()

        BEGIN_STATE(SelfClosingStartTag)
          // TODO-STATE
        END_STATE()
      }
    }

    KRYS_NODISCARD bool EmitEOFToken() noexcept
    {
      _state = TokenizerState::Data;

      _token.Clear();
      _token.SetAsEOF();

      return true;
    }
  };
}