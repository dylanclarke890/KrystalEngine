#include "Krystal.Booey/Core/DateTimeFormat.hpp"
#include "Krystal.Core/Assert.hpp"
#include <array>

namespace krys::boo
{
  namespace
  {
    using FieldType = DateTimeFormat::FieldType;

    constexpr std::array LowerCaseToFieldTypeMap {
      FieldType::Period,                   // a
      FieldType::Invalid,                  // b
      FieldType::LocalDayOfWeekStandAlone, // c
      FieldType::DayOfMonth,               // d
      FieldType::LocalDayOfWeek,           // e
      FieldType::Invalid,                  // f
      FieldType::ModifiedJulianDay,        // g
      FieldType::Hour12,                   // h
      FieldType::Invalid,                  // i
      FieldType::Invalid,                  // j
      FieldType::Hour24,                   // k
      FieldType::Invalid,                  // l
      FieldType::Minute,                   // m
      FieldType::Invalid,                  // n
      FieldType::Invalid,                  // o
      FieldType::Invalid,                  // p
      FieldType::QuarterStandAlone,        // q
      FieldType::Invalid,                  // r
      FieldType::Second,                   // s
      FieldType::Invalid,                  // t
      FieldType::ExtendedYear,             // u
      FieldType::NonLocationZone,          // v
      FieldType::WeekOfYear,               // w
      FieldType::Invalid,                  // x
      FieldType::Year,                     // y
      FieldType::Zone,                     // z
    };

    constexpr std::array UpperCaseToFieldTypeMap {
      FieldType::MillisecondsInDay, // A
      FieldType::Invalid,           // B
      FieldType::Invalid,           // C
      FieldType::DayOfYear,         // D
      FieldType::DayOfWeek,         // E
      FieldType::DayOfWeekInMonth,  // F
      FieldType::Era,               // G
      FieldType::Hour23,            // H
      FieldType::Invalid,           // I
      FieldType::Invalid,           // J
      FieldType::Hour11,            // K
      FieldType::MonthStandAlone,   // L
      FieldType::Month,             // M
      FieldType::Invalid,           // N
      FieldType::Invalid,           // O
      FieldType::Invalid,           // P
      FieldType::Quarter,           // Q
      FieldType::Invalid,           // R
      FieldType::FractionalSecond,  // S
      FieldType::Invalid,           // T
      FieldType::Invalid,           // U
      FieldType::Invalid,           // V
      FieldType::WeekOfMonth,       // W
      FieldType::Invalid,           // X
      FieldType::YearOfWeekOfYear,  // Y
      FieldType::RFC822Zone,        // Z
    };

    KRYS_NODISCARD FieldType MapCharacterToFieldType(char8 ch) noexcept
    {
      if (ch >= 'A' && ch <= 'Z')
      {
        return UpperCaseToFieldTypeMap[static_cast<size_t>(ch - 'A')];
      }

      if (ch >= 'a' && ch <= 'z')
      {
        return LowerCaseToFieldTypeMap[static_cast<size_t>(ch - 'a')];
      }

      return FieldType::Literal;
    }

    KRYS_NODISCARD bool IsASCIIAlphabetOrQuote(char8 ch) noexcept
    {
      return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '\'';
    }
  }

  bool DateTimeFormat::Parse(utf8_stringview source, TokenHandler &tokenHandler) noexcept
  {
    enum class State : uint8
    {
      InQuote,
      InQuoteQuote,
      Literal,
      Quote,
      Symbol,
    } state = State::Literal;

    FieldType fieldType = FieldType::Literal;
    utf8_string literalBuffer;
    int fieldCounter = 0;

    // Bytes of non-ASCII characters are literals, so they're kept whole in the literal buffer.
    for (char8 ch : source)
    {
      switch (state)
      {
        case State::InQuote:
          if (ch == '\'')
          {
            state = State::InQuoteQuote;
            break;
          }

          literalBuffer.push_back(ch);
          break;

        case State::InQuoteQuote:
          if (ch == '\'')
          {
            literalBuffer.push_back('\'');
            state = State::InQuote;
            break;
          }

          fieldType = MapCharacterToFieldType(ch);
          if (fieldType == FieldType::Invalid)
          {
            return false;
          }

          if (fieldType == FieldType::Literal)
          {
            literalBuffer.push_back(ch);
            state = State::Literal;
            break;
          }

          if (!literalBuffer.empty())
          {
            tokenHandler.VisitLiteral(literalBuffer);
            literalBuffer.clear();
          }

          fieldCounter = 1;
          state = State::Symbol;
          break;

        case State::Literal:
          if (ch == '\'')
          {
            state = State::Quote;
            break;
          }

          fieldType = MapCharacterToFieldType(ch);
          if (fieldType == FieldType::Invalid)
          {
            return false;
          }

          if (fieldType == FieldType::Literal)
          {
            literalBuffer.push_back(ch);
            break;
          }

          if (!literalBuffer.empty())
          {
            tokenHandler.VisitLiteral(literalBuffer);
            literalBuffer.clear();
          }

          fieldCounter = 1;
          state = State::Symbol;
          break;

        case State::Quote:
          literalBuffer.push_back(ch);
          state = ch == '\'' ? State::Literal : State::InQuote;
          break;

        case State::Symbol:
        {
          krys_debug_assert(fieldType != FieldType::Invalid);
          krys_debug_assert(fieldType != FieldType::Literal);
          krys_debug_assert(literalBuffer.empty());

          FieldType fieldType2 = MapCharacterToFieldType(ch);
          if (fieldType2 == FieldType::Invalid)
          {
            return false;
          }

          if (fieldType == fieldType2)
          {
            ++fieldCounter;
            break;
          }

          tokenHandler.VisitField(fieldType, fieldCounter);

          if (fieldType2 == FieldType::Literal)
          {
            if (ch == '\'')
            {
              state = State::Quote;
            }
            else
            {
              literalBuffer.push_back(ch);
              state = State::Literal;
            }
            break;
          }

          fieldCounter = 1;
          fieldType = fieldType2;
          break;
        }
      }
    }

    krys_debug_assert(fieldType != FieldType::Invalid);

    switch (state)
    {
      case State::Literal:
      case State::InQuoteQuote:
        if (!literalBuffer.empty())
        {
          tokenHandler.VisitLiteral(literalBuffer);
        }
        return true;

      case State::Quote:
      case State::InQuote:
        if (!literalBuffer.empty())
        {
          tokenHandler.VisitLiteral(literalBuffer);
        }
        return false;

      case State::Symbol:
        krys_debug_assert(fieldType != FieldType::Literal);
        krys_debug_assert(literalBuffer.empty());
        tokenHandler.VisitField(fieldType, fieldCounter);
        return true;
    }

    krys_debug_assert(false);
    return false;
  }

  void DateTimeFormat::QuoteAndAppendLiteral(utf8_stringview literal, utf8_string &buffer) noexcept
  {
    if (literal.empty())
    {
      return;
    }

    bool hasAlphabetOrQuote = false;
    for (auto ch : literal)
    {
      if (IsASCIIAlphabetOrQuote(ch))
      {
        hasAlphabetOrQuote = true;
        break;
      }
    }
    if (!hasAlphabetOrQuote)
    {
      buffer.append(literal);
      return;
    }

    if (literal.find('\'') == utf8_stringview::npos)
    {
      buffer.push_back('\'');
      buffer.append(literal);
      buffer.push_back('\'');
      return;
    }

    for (size_t i = 0; i < literal.size(); ++i)
    {
      if (literal[i] == '\'')
      {
        buffer.append(u8"''");
      }
      else
      {
        buffer.push_back('\'');
        for (auto ch : literal.substr(i))
        {
          if (ch == '\'')
          {
            buffer.append(u8"''");
          }
          else
          {
            buffer.push_back(ch);
          }
        }
        buffer.push_back('\'');
        return;
      }
    }
  }
}
