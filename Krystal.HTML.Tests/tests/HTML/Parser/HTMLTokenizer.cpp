#include "Krystal.HTML/HTML/Parser/HTMLTokenizer.hpp"
#include "Krystal.HTML.Tests/TestParserUtils.hpp"
#include "Krystal.HTML/HTML/Enums/HTMLParseError.hpp"
#include "Krystal.HTML/HTML/Enums/TokenizerState.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLToken.hpp"
#include "Krystal.Lib/Core/MagicEnum.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Text/StringConversion.hpp"

namespace Krys::HTML::Tests
{
  namespace
  {
    void SerializeHTMLToken(HTMLToken &token, DOMString &output) noexcept
    {
      switch (token.Type())
      {
        case HTMLTokenType::DOCTYPE:
        {
          auto doctype = token.ReleaseDOCTYPEData();
          output += u8"DOCTYPE(";
          output += token.Name();
          output += u8", ";
          output += DOMStringView(doctype->PublicIdentifier);
          output += u8", ";
          output += DOMStringView(doctype->SystemIdentifier);
          output += u8", ";
          output += u8"ForceQuirks=";
          output += doctype->ForceQuirks ? u8"true" : u8"false";
          output += u8")\n";
          break;
        }
        case HTMLTokenType::Character:
        {
          output += u8"Character(";
          output += DOMStringView(token.Data());
          output += u8")\n";
          break;
        }
        case HTMLTokenType::Comment:
        {
          output += u8"Comment(";
          output += DOMStringView(token.Data());
          output += u8")\n";
          break;
        }
        case HTMLTokenType::StartTag:
        case HTMLTokenType::EndTag:
        {
          if (token.Type() == HTMLTokenType::StartTag)
          {
            output += u8"StartTag(";
          }
          else
          {
            output += u8"EndTag(";
          }

          output += token.Name();

          for (const auto &attribute : token.Attributes())
          {
            output += u8", ";
            output += attribute.NameView();
            output += u8"=\"";
            output += attribute.ValueView();
            output += u8"\"";
          }

          if (token.Type() == HTMLTokenType::StartTag)
          {
            output += u8", SelfClosing=";
            output += token.IsSelfClosing() ? u8"true" : u8"false";
          }

          output += u8")\n";
          break;
        }
        case HTMLTokenType::EndOfFile:
        {
          output += u8"EndOfFile\n";
          break;
        }
      }
    }

    struct HTMLTokenizerTest
    {
      string Name;
      utf8_string Html;
      utf8_string Tokens;
      TokenizerState InitialState {TokenizerState::Data};
      TokenizerState ExpectedState {TokenizerState::Data};
      bool AppendEOF {false};
      bool CDATASectionAllowed {false};
      List<HTMLTokenizerError> Errors {};
    };

    KRYS_NODISCARD List<HTMLTokenizerTest> ParseHTMLTokenizerTests(std::istream &stream) noexcept
    {
      List<HTMLTokenizerTest> tests;
      HTMLTokenizerTest currentTest;

      auto parse = [&](utf8_string sectionName, utf8_string data)
      {
        if (sectionName == u8"name")
        {
          if (!currentTest.Name.empty() && !currentTest.Html.empty())
          {
            tests.push_back(Krys::Move(currentTest));
            currentTest = {};
          }

          currentTest.Name = string(data.begin(), data.end());
        }
        else if (sectionName == u8"data")
        {
          NormaliseData(data);
          currentTest.Html = Krys::Move(data);
        }
        else if (sectionName == u8"tokens")
        {
          NormaliseData(data);
          currentTest.Tokens = Krys::Move(data);
        }
        else if (sectionName == u8"initial-state")
        {
          NormaliseData(data);

          auto state = magic_enum::enum_cast<TokenizerState>(
            stringview(reinterpret_cast<const char *>(data.data()), data.size()),
            magic_enum::case_insensitive);
          if (state.has_value())
          {
            currentTest.InitialState = *state;
          }
        }
        else if (sectionName == u8"expected-state")
        {
          NormaliseData(data);

          auto state = magic_enum::enum_cast<TokenizerState>(
            stringview(reinterpret_cast<const char *>(data.data()), data.size()),
            magic_enum::case_insensitive);
          if (state.has_value())
          {
            currentTest.ExpectedState = *state;
          }
        }
        else if (sectionName == u8"append-eof")
        {
          currentTest.AppendEOF = true;
        }
        else if (sectionName == u8"cdata-allowed")
        {
          currentTest.CDATASectionAllowed = true;
        }
        else if (sectionName == u8"errors")
        {
          auto lines = SplitStringByNewline(data);

          for (const auto &line : lines)
          {
            auto text = stringview(line);
            // NOTE: errors are in the form "line:column ErrorName"

            uint32 line = 0u;
            auto [ptr1, ec1] = std::from_chars(text.data(), text.data() + text.size(), line);
            text.remove_prefix(ptr1 - text.data());

            text.remove_prefix(1uz); // skip ':'

            uint32 column = 0u;
            auto [ptr2, ec2] = std::from_chars(text.data(), text.data() + text.size(), column);
            text.remove_prefix(ptr2 - text.data());

            text.remove_prefix(1uz); // skip ' '

            auto error = magic_enum::enum_cast<HTMLParseError>(text, magic_enum::case_insensitive);
            if (error.has_value())
            {
              currentTest.Errors.push_back({
                .Error = *error,
                .Location =
                  {
                    .Line = line,
                    .Column = column,
                  },
              });
            }
          }
        }
      };

      ParseTestData(stream, "#", ::Krys::Move(parse));
      if (!currentTest.Html.empty())
      {
        tests.push_back(Krys::Move(currentTest));
      }

      return tests;
    }

    void ExecuteHTMLTokenizerTest(const HTMLTokenizerTest &test, size_t number, size_t total) noexcept
    {
      HTMLInputStream inputStream;
      HTMLTokenizer tokenizer(inputStream, [allowed = test.CDATASectionAllowed]() { return allowed; });
      tokenizer.State(test.InitialState);
      if (test.AppendEOF)
      {
        inputStream.Append(utf8_string(test.Html), IsEOF(true));
      }
      else
      {
        inputStream.Append(utf8_string(test.Html), IsEOF(false));
      }

      DOMString tokens;
      while (true)
      {
        auto token = tokenizer.NextToken();
        if (!token)
        {
          break;
        }
        SerializeHTMLToken(*token, tokens);
      }

      NormaliseData(tokens);

      utf8_string str = u8"--- TEST " + ToUTF8(number + 1uz) + u8" OF " + ToUTF8(total) + u8" ---\n"
                        + ToUTF8(test.Name.empty() ? "Unnamed Test" : test.Name);
      UTF8_INFO(str);

      str = u8"\n--- INPUT ---\n" + test.Html;
      UTF8_INFO(str);

      str = u8"--- EXPECTED TOKENS ---\n" + test.Tokens;
      UTF8_INFO(str);

      str = u8"\n--- ACTUAL TOKENS ---\n" + tokens;
      UTF8_INFO(str);

      auto initialState = string(magic_enum::enum_name(test.InitialState));
      str = u8"\n--- INITIAL STATE ---\n" + ToUTF8(initialState);

      auto expectedState = string(magic_enum::enum_name(test.ExpectedState));
      str = u8"\n--- EXPECTED STATE ---\n" + ToUTF8(expectedState);

      CHECK(tokenizer.State() == test.ExpectedState);

      size_t errorIndex = 0uz;

      for (const auto &error : tokenizer.ParseErrors())
      {
        if (errorIndex >= test.Errors.size())
        {
          break;
        }

        CHECK(error.Error == test.Errors[errorIndex].Error);
        CHECK(error.Location.Line == test.Errors[errorIndex].Location.Line);
        CHECK(error.Location.Column == test.Errors[errorIndex].Location.Column);
        ++errorIndex;
      }

      bool equal = tokens == test.Tokens;
      CHECK(equal);
    }

    void RunTest(string filename) noexcept
    {
      static string basedir = "data/html-tokenizer/";

      auto file = OpenTestDataFile(basedir + filename);
      REQUIRE(file.has_value());

      auto tests = ParseHTMLTokenizerTests(*file);
      REQUIRE(!tests.empty());

      ExecuteTests(tests, ExecuteHTMLTokenizerTest);
    }
  }

#define EXECUTE_HTML_TOKENIZER_TEST_CASE(FileName)                                                           \
  TEST_CASE("HTMLTokenizer(" FileName ")", "[HTML][HTMLTokenizer]")                                          \
  {                                                                                                          \
    RunTest(FileName);                                                                                       \
  }

  EXECUTE_HTML_TOKENIZER_TEST_CASE("afterattributename-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("afterattributevaluequoted-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("afterdoctypename-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("afterdoctypepublicidentifier-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("afterdoctypepublickeyword-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("afterdoctypesystemidentifier-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("afterdoctypesystemkeyword-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("attributename-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("attributevaluedoublequoted-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("attributevaluesinglequoted-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("attributevalueunquoted-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("beforeattributename-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("beforeattributevalue-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("beforedoctypename-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("beforedoctypepublicidentifier-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("beforedoctypesystemidentifier-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("betweendoctypepublicandsystemidentifiers-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("boguscomment-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("bogusdoctype-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("cdatasection-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("cdatasectionbracket-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("cdatasectionend-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("character-reference-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("character-reference-02.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("character-reference-03.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("character-reference-04.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("comment-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("commentend-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("commentendbang-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("commentenddash-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("commentlessthan-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("commentlessthanbang-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("commentlessthanbangdash-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("commentlessthanbangdashdash-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("commentstart-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("commentstartdash-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("data-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("doctype-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("doctypename-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("doctypepublicidentifierdoublequoted-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("doctypepublicidentifiersinglequoted-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("doctypesystemidentifierdoublequoted-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("doctypesystemidentifiersinglequoted-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("endtagopen-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("markupdeclarationopen-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("plaintext-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("rawtext-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("rawtextendtagopen-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("rawtextlessthan-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("rcdata-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("rcdataendtagname-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("rcdataendtagopen-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("rcdatalessthan-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdata-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescaped-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescapeddash-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescapeddashdash-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescapedlessthan-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescapeend-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescapestart-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataendtagname-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataendtagopen-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataescaped-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataescapeddash-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataescapeddashdash-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataescapedendtagname-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataescapedendtagopen-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataescapedlessthan-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataescapestart-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdatalessthan-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("selfclosingstartag-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("tagname-01.dat");
  EXECUTE_HTML_TOKENIZER_TEST_CASE("tagopen-01.dat");

  EXECUTE_HTML_TOKENIZER_TEST_CASE("integration-test-01.dat");
}