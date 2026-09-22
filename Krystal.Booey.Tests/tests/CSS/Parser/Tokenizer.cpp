#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey.Tests/TestParserUtils.hpp"
#include "Krystal.Booey/CSS/Parser/ParseError.hpp"
#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::css::tests
{
  namespace
  {

#pragma region Serialize

    void SerializeNumericValue(const Token &token, dom::DOMString &output) noexcept
    {
      output += krys::boo::tests::ToUTF8(token.NumericValue());
    }

    void SerializeCSSToken(const Token &token, dom::DOMString &output) noexcept
    {
      switch (token.Type())
      {
        case TokenType::Ident:
        {
          output += u8"Ident(";
          output += token.IdentCodePoints();
          output += u8")\n";
          break;
        }
        case TokenType::Function:
        {
          output += u8"Function(";
          output += token.IdentCodePoints();
          output += u8")\n";
          break;
        }
        case TokenType::AtKeyword:
        {
          output += u8"AtKeyword(";
          output += token.IdentCodePoints();
          output += u8")\n";
          break;
        }
        case TokenType::Hash:
        {
          output += u8"Hash(";
          output += token.IdentCodePoints();
          output += u8", ";
          output += (token.HashType() == HashTokenType::Id ? u8"Id" : u8"Unrestricted");
          output += u8")\n";
          break;
        }
        case TokenType::String:
        {
          output += u8"String(";
          output += token.IdentCodePoints();
          output += u8")\n";
          break;
        }
        case TokenType::BadString:
        {
          output += u8"BadString\n";
          break;
        }
        case TokenType::Url:
        {
          output += u8"Url(";
          output += token.IdentCodePoints();
          output += u8")\n";
          break;
        }
        case TokenType::BadUrl:
        {
          output += u8"BadUrl\n";
          break;
        }
        case TokenType::Delim:
        {
          output += u8"Delim(";
          output += token.IdentCodePoints();
          output += u8")\n";
          break;
        }
        case TokenType::Number:
        {
          output += u8"Number(";
          SerializeNumericValue(token, output);
          output += u8")\n";
          break;
        }
        case TokenType::Percentage:
        {
          output += u8"Percentage(";
          SerializeNumericValue(token, output);
          output += u8")\n";
          break;
        }
        case TokenType::Dimension:
        {
          output += u8"Dimension(";
          SerializeNumericValue(token, output);
          output += u8", ";
          output += token.Unit();
          output += u8")\n";
          break;
        }
        case TokenType::Whitespace:
        {
          output += u8"Whitespace\n";
          break;
        }
        case TokenType::CDO:
        {
          output += u8"CDO\n";
          break;
        }
        case TokenType::CDC:
        {
          output += u8"CDC\n";
          break;
        }
        case TokenType::Colon:
        {
          output += u8"Colon\n";
          break;
        }
        case TokenType::Semicolon:
        {
          output += u8"Semicolon\n";
          break;
        }
        case TokenType::Comma:
        {
          output += u8"Comma\n";
          break;
        }
        case TokenType::OpenSquare:
        {
          output += u8"OpenSquare\n";
          break;
        }
        case TokenType::CloseSquare:
        {
          output += u8"CloseSquare\n";
          break;
        }
        case TokenType::OpenParen:
        {
          output += u8"OpenParen\n";
          break;
        }
        case TokenType::CloseParen:
        {
          output += u8"CloseParen\n";
          break;
        }
        case TokenType::OpenCurly:
        {
          output += u8"OpenCurly\n";
          break;
        }
        case TokenType::CloseCurly:
        {
          output += u8"CloseCurly\n";
          break;
        }
      }
    }

    KRYS_NODISCARD dom::DOMString SerializeCSSTokens(TokenRange tokens) noexcept
    {
      dom::DOMString output;

      for (auto &token : tokens)
      {
        SerializeCSSToken(token, output);
      }

      krys::boo::tests::NormaliseData(output);
      return output;
    }

#pragma endregion

    struct CSSTokenizerTest
    {
      utf8_string Css;
      utf8_string Tokens;
      List<TokenizerError> Errors;
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
            krys::boo::tests::NormaliseData(currentTest.Tokens);
            tests.push_back(::krys::move(currentTest));
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

      krys::boo::tests::ParseTestData(stream, "$", ::krys::move(parse));
      if (!currentTest.Css.empty())
      {
        tests.push_back(::krys::move(currentTest));
      }

      return tests;
    }

    void ExecuteCSSTokenizerTest(const CSSTokenizerTest &test, size_t number, size_t total) noexcept
    {
      utf8_string str = u8"--- TEST " + krys::boo::tests::ToUTF8(number + 1uz) + u8" OF "
                        + krys::boo::tests::ToUTF8(total) + u8" ---\n" + test.Css;
      UTF8_INFO(str);

      str = u8"--- EXPECTED TOKENS ---\n" + test.Tokens;
      UTF8_INFO(str);

      InputStream inputStream(utf8_string(test.Css));
      Tokenizer tokenizer(inputStream);
      REQUIRE(tokenizer.PumpTokenizer());

      auto actual = SerializeCSSTokens(tokenizer.Tokens());
      str = u8"\n--- ACTUAL TOKENS ---\n" + actual;
      UTF8_INFO(str);

      bool equal = actual == test.Tokens;
      CHECK(equal);
    }

    void RunTest(string filename) noexcept
    {
      static string basedir = KRYS_BOOEY_TESTS_DATA_DIR "/css-tokenizer/";

      auto file = krys::boo::tests::OpenTestDataFile(basedir + filename);
      REQUIRE(file.has_value());

      auto tests = ParseCSSTokenizerTests(*file);
      REQUIRE(!tests.empty());

      krys::boo::tests::ExecuteTests(tests, ExecuteCSSTokenizerTest);
    }
  }

#define EXECUTE_CSS_TOKENIZER_TEST_CASE(FileName)                                                            \
  TEST_CASE("Tokenizer(" FileName ")", "[HTML][Tokenizer]")                                                  \
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