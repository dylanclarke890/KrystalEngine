#pragma once

#include "Krystal.HTML.Tests/TestParsers/TestParserUtils.hpp"
#include "Krystal.HTML/CSS/Parser/CSSToken.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenizer.hpp"
#include "Krystal.HTML/CSS/Parser/Enums/CSSParseError.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Text/StringConversion.hpp"

namespace Krys::HTML::Tests
{
  struct CSSTokenizerTest
  {
    utf32_string Css;
    utf8_string Tokens;
    List<CSSTokenizerError> Errors;
  };

  enum class CSSTokenizerTestSection
  {
    None,
    Data,
    Tokens,
    Errors,
  };

  inline void SerializeNumericValue(const CSSToken &token, DOMString &output)
  {
    auto numericValue = token.NumericValue();
    if (token.NumericTokenType() == NumericTokenType::Integer)
    {
      output += ToUTF8(std::get<int64>(numericValue));
    }
    else
    {
      output += ToUTF8(std::get<double>(numericValue));
    }
  }

  inline void SerializeCSSToken(const CSSToken &token, DOMString &output) noexcept
  {
    switch (token.Type())
    {
      case CSSTokenType::Ident:
      {
        output += u8"Ident(";
        output += Krys::Text::ConvertToUTF8(token.IdentCodePoints());
        output += u8")\n";
        break;
      }
      case CSSTokenType::Function:
      {
        output += u8"Function(";
        output += Krys::Text::ConvertToUTF8(token.IdentCodePoints());
        output += u8")\n";
        break;
      }
      case CSSTokenType::AtKeyword:
      {
        output += u8"AtKeyword(";
        output += Krys::Text::ConvertToUTF8(token.IdentCodePoints());
        output += u8")\n";
        break;
      }
      case CSSTokenType::Hash:
      {
        output += u8"Hash(";
        output += Krys::Text::ConvertToUTF8(token.IdentCodePoints());
        output += u8")\n";
        break;
      }
      case CSSTokenType::String:
      {
        output += u8"String(";
        output += Krys::Text::ConvertToUTF8(token.IdentCodePoints());
        output += u8")\n";
        break;
      }
      case CSSTokenType::BadString:
      {
        output += u8"BadString\n";
        break;
      }
      case CSSTokenType::Url:
      {
        output += u8"Url(";
        output += Krys::Text::ConvertToUTF8(token.IdentCodePoints());
        output += u8")\n";
        break;
      }
      case CSSTokenType::BadUrl:
      {
        output += u8"BadUrl\n";
        break;
      }
      case CSSTokenType::Delim:
      {
        output += u8"Delim(";
        output += Krys::Text::ConvertToUTF8(token.IdentCodePoints());
        output += u8")\n";
        break;
      }
      case CSSTokenType::Number:
      {
        output += u8"Number(";
        SerializeNumericValue(token, output);
        output += u8")\n";
        break;
      }
      case CSSTokenType::Percentage:
      {
        output += u8"Percentage(";
        SerializeNumericValue(token, output);
        output += u8")\n";
        break;
      }
      case CSSTokenType::Dimension:
      {
        output += u8"Dimension(";
        SerializeNumericValue(token, output);
        output += u8", ";
        output += Krys::Text::ConvertToUTF8(token.Unit());
        output += u8")\n";
        break;
      }
      case CSSTokenType::Whitespace:
      {
        output += u8"Whitespace\n";
        break;
      }
      case CSSTokenType::CDO:
      {
        output += u8"CDO\n";
        break;
      }
      case CSSTokenType::CDC:
      {
        output += u8"CDC\n";
        break;
      }
      case CSSTokenType::Colon:
      {
        output += u8"Colon\n";
        break;
      }
      case CSSTokenType::Semicolon:
      {
        output += u8"Semicolon\n";
        break;
      }
      case CSSTokenType::Comma:
      {
        output += u8"Comma\n";
        break;
      }
      case CSSTokenType::OpenSquare:
      {
        output += u8"OpenSquare\n";
        break;
      }
      case CSSTokenType::CloseSquare:
      {
        output += u8"CloseSquare\n";
        break;
      }
      case CSSTokenType::OpenParen:
      {
        output += u8"OpenParen\n";
        break;
      }
      case CSSTokenType::CloseParen:
      {
        output += u8"CloseParen\n";
        break;
      }
      case CSSTokenType::OpenCurly:
      {
        output += u8"OpenCurly\n";
        break;
      }
      case CSSTokenType::CloseCurly:
      {
        output += u8"CloseCurly\n";
        break;
      }
    }
  }

  KRYS_NODISCARD inline DOMString SerializeCSSTokens(CSSTokenRange tokens) noexcept
  {
    DOMString output;

    for (auto &token : tokens)
    {
      SerializeCSSToken(token, output);
    }

    NormaliseData(output);
    return output;
  }

  KRYS_NODISCARD inline List<CSSTokenizerTest> ParseCSSTokenizerTests(std::istream &stream) noexcept
  {
    List<CSSTokenizerTest> tests;

    string css;
    string tokens;

    auto FinishParsingTest = [&]()
    {
      NormaliseData(css);
      NormaliseData(tokens);

      tests.push_back({
        .Css = Krys::Text::ConvertToUTF32(utf8_stringview(ToUTF8(css))),
        .Tokens = ToUTF8(tokens),
      });

      css.clear();
      tokens.clear();
    };

    string line;
    CSSTokenizerTestSection section = CSSTokenizerTestSection::None;
    while (std::getline(stream, line))
    {
      if (!line.empty() && line.back() == '\r')
      {
        line.pop_back();
      }

      if (line == "#data")
      {
        NormaliseData(tokens);

        if (!css.empty() && !tokens.empty())
        {
          FinishParsingTest();
        }

        section = CSSTokenizerTestSection::Data;
        continue;
      }

      if (line == "#errors")
      {
        section = CSSTokenizerTestSection::Errors;
        continue;
      }

      if (line == "#tokens")
      {
        section = CSSTokenizerTestSection::Tokens;
        continue;
      }

      switch (section)
      {
        case CSSTokenizerTestSection::None:
        {
          continue;
        }
        case CSSTokenizerTestSection::Data:
        {
          css += line;
          css += '\n';
          continue;
        }
        case CSSTokenizerTestSection::Tokens:
        {
          tokens += line;
          tokens += '\n';
          continue;
        }
        case CSSTokenizerTestSection::Errors:
        {
          // TODO: parse errors
          continue;
        }
      }
    }

    if (!css.empty() && !tokens.empty())
    {
      FinishParsingTest();
    }

    return tests;
  }

  KRYS_NODISCARD inline Maybe<List<CSSTokenizerTest>> ParseCSSTokenizerTests(const string &filePath) noexcept
  {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
      return {};
    }

    SkipUTF8FileBOM(file);
    return ParseCSSTokenizerTests(file);
  }
}