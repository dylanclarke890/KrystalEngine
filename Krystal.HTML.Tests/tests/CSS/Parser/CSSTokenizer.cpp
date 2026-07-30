#include "Krystal.HTML/CSS/Parser/CSSTokenizer.hpp"
#include "Krystal.HTML.Tests/TestParserUtils.hpp"
#include "Krystal.HTML/CSS/Parser/CSSToken.hpp"
#include "Krystal.HTML/CSS/Parser/Enums/CSSParseError.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Text/StringConversion.hpp"
#include <catch_all.hpp>

namespace Krys::HTML::Tests
{
  namespace
  {

#pragma region Serialize

    void SerializeNumericValue(const CSSToken &token, DOMString &output) noexcept
    {
      output += ToUTF8(token.NumericValue());
    }

    void SerializeCSSToken(const CSSToken &token, DOMString &output) noexcept
    {
      switch (token.Type())
      {
        case CSSTokenType::Ident:
        {
          output += u8"Ident(";
          output += token.IdentCodePoints();
          output += u8")\n";
          break;
        }
        case CSSTokenType::Function:
        {
          output += u8"Function(";
          output += token.IdentCodePoints();
          output += u8")\n";
          break;
        }
        case CSSTokenType::AtKeyword:
        {
          output += u8"AtKeyword(";
          output += token.IdentCodePoints();
          output += u8")\n";
          break;
        }
        case CSSTokenType::Hash:
        {
          output += u8"Hash(";
          output += token.IdentCodePoints();
          output += u8", ";
          output += (token.HashType() == HashTokenType::Id ? u8"Id" : u8"Unrestricted");
          output += u8")\n";
          break;
        }
        case CSSTokenType::String:
        {
          output += u8"String(";
          output += token.IdentCodePoints();
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
          output += token.IdentCodePoints();
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
          output += token.IdentCodePoints();
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
          output += token.Unit();
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

    KRYS_NODISCARD DOMString SerializeCSSTokens(CSSTokenRange tokens) noexcept
    {
      DOMString output;

      for (auto &token : tokens)
      {
        SerializeCSSToken(token, output);
      }

      NormaliseData(output);
      return output;
    }

#pragma endregion

    struct CSSTokenizerTest
    {
      utf8_string Css;
      utf8_string Tokens;
      List<CSSTokenizerError> Errors;
    };

    KRYS_NODISCARD List<CSSTokenizerTest> ParseCSSTokenizerTests(std::istream &stream) noexcept
    {
      List<CSSTokenizerTest> tests;
      CSSTokenizerTest currentTest;

      auto parse = [&](utf8_string sectionName, utf8_string data)
      {
        if (sectionName == u8"data")
        {
          if (!currentTest.Css.empty())
          {
            NormaliseData(currentTest.Tokens);
            tests.push_back(::Krys::Move(currentTest));
            currentTest = {};
          }

          currentTest.Css = data;
        }
        else if (sectionName == u8"tokens")
        {
          currentTest.Tokens = data;
        }
        else if (sectionName == u8"errors")
        {
        }
      };

      ParseTestData(stream, "$", ::Krys::Move(parse));
      if (!currentTest.Css.empty())
      {
        tests.push_back(::Krys::Move(currentTest));
      }

      return tests;
    }

    void ExecuteCSSTokenizerTest(const CSSTokenizerTest &test, size_t number, size_t total) noexcept
    {
      utf8_string str =
        u8"--- TEST " + ToUTF8(number + 1uz) + u8" OF " + ToUTF8(total) + u8" ---\n" + test.Css;
      UTF8_INFO(str);

      str = u8"--- EXPECTED TOKENS ---\n" + test.Tokens;
      UTF8_INFO(str);

      CSSInputStream inputStream(utf8_string(test.Css));
      CSSTokenizer tokenizer(inputStream);
      REQUIRE(tokenizer.PumpTokenizer());

      auto actual = SerializeCSSTokens(tokenizer.TokenRange());
      str = u8"\n--- ACTUAL TOKENS ---\n" + actual;
      UTF8_INFO(str);

      bool equal = actual == test.Tokens;
      CHECK(equal);
    }

    void RunTest(string filename) noexcept
    {
      static string basedir = "data/css-tokenizer/";

      auto file = OpenTestDataFile(basedir + filename);
      REQUIRE(file.has_value());

      auto tests = ParseCSSTokenizerTests(*file);
      REQUIRE(!tests.empty());

      ExecuteTests(tests, ExecuteCSSTokenizerTest);
    }
  }

#define EXECUTE_CSS_TOKENIZER_TEST_CASE(FileName)                                                            \
  TEST_CASE("CSSTokenizer(" FileName ")", "[HTML][CSSTokenizer]")                                            \
  {                                                                                                          \
    RunTest(FileName);                                                                                       \
  }

  EXECUTE_CSS_TOKENIZER_TEST_CASE("at-keyword-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("bad-string-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("bad-url-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("close-curly-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("close-paren-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("close-square-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("colon-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("comma-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("digit-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("dimension-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("escaped-code-point-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("full-stop-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("fuzz-01.dat");
  EXECUTE_CSS_TOKENIZER_TEST_CASE("fuzz-02.dat");
  EXECUTE_CSS_TOKENIZER_TEST_CASE("fuzz-03.dat");
  EXECUTE_CSS_TOKENIZER_TEST_CASE("fuzz-04.dat");
  EXECUTE_CSS_TOKENIZER_TEST_CASE("fuzz-05.dat");
  EXECUTE_CSS_TOKENIZER_TEST_CASE("fuzz-06.dat");
  EXECUTE_CSS_TOKENIZER_TEST_CASE("fuzz-07.dat");
  EXECUTE_CSS_TOKENIZER_TEST_CASE("fuzz-08.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("hash-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("hyphen-minus-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("ident-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("ident-like-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("less-than-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("number-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("numeric-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("open-curly-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("open-paren-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("open-square-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("plus-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("reverse-solidus-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("semicolon-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("string-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("url-01.dat");
}