#include "Krystal.Booey/Html/Parser/Tokenizer.hpp"
#include "Krystal.Booey.Tests/TestParserUtils.hpp"
#include "Krystal.Booey/Html/Parser/Token.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <algorithm>
#include <utility>

namespace krys::boo::html::tests
{
  namespace
  {
    // The tokenizer tests name states and errors by their enumerators, in any case.
    constexpr std::pair<stringview, TokenizerState> TokenizerTestStateNames[] = {
      {"Data", TokenizerState::Data},
      {"RCDATA", TokenizerState::RCDATA},
      {"RAWTEXT", TokenizerState::RAWTEXT},
      {"ScriptData", TokenizerState::ScriptData},
      {"PLAINTEXT", TokenizerState::PLAINTEXT},
      {"TagOpen", TokenizerState::TagOpen},
      {"EndTagOpen", TokenizerState::EndTagOpen},
      {"TagName", TokenizerState::TagName},
      {"RCDATALessThanSign", TokenizerState::RCDATALessThanSign},
      {"RCDATAEndTagOpen", TokenizerState::RCDATAEndTagOpen},
      {"RCDATAEndTagName", TokenizerState::RCDATAEndTagName},
      {"RAWTEXTLessThanSign", TokenizerState::RAWTEXTLessThanSign},
      {"RAWTEXTEndTagOpen", TokenizerState::RAWTEXTEndTagOpen},
      {"RAWTEXTEndTagName", TokenizerState::RAWTEXTEndTagName},
      {"ScriptDataLessThanSign", TokenizerState::ScriptDataLessThanSign},
      {"ScriptDataEndTagOpen", TokenizerState::ScriptDataEndTagOpen},
      {"ScriptDataEndTagName", TokenizerState::ScriptDataEndTagName},
      {"ScriptDataEscapeStart", TokenizerState::ScriptDataEscapeStart},
      {"ScriptDataEscapeStartDash", TokenizerState::ScriptDataEscapeStartDash},
      {"ScriptDataEscaped", TokenizerState::ScriptDataEscaped},
      {"ScriptDataEscapedDash", TokenizerState::ScriptDataEscapedDash},
      {"ScriptDataEscapedDashDash", TokenizerState::ScriptDataEscapedDashDash},
      {"ScriptDataEscapedLessThanSign", TokenizerState::ScriptDataEscapedLessThanSign},
      {"ScriptDataEscapedEndTagOpen", TokenizerState::ScriptDataEscapedEndTagOpen},
      {"ScriptDataEscapedEndTagName", TokenizerState::ScriptDataEscapedEndTagName},
      {"ScriptDataDoubleEscapeStart", TokenizerState::ScriptDataDoubleEscapeStart},
      {"ScriptDataDoubleEscaped", TokenizerState::ScriptDataDoubleEscaped},
      {"ScriptDataDoubleEscapedDash", TokenizerState::ScriptDataDoubleEscapedDash},
      {"ScriptDataDoubleEscapedDashDash", TokenizerState::ScriptDataDoubleEscapedDashDash},
      {"ScriptDataDoubleEscapedLessThanSign", TokenizerState::ScriptDataDoubleEscapedLessThanSign},
      {"ScriptDataDoubleEscapeEnd", TokenizerState::ScriptDataDoubleEscapeEnd},
      {"BeforeAttributeName", TokenizerState::BeforeAttributeName},
      {"AttributeName", TokenizerState::AttributeName},
      {"AfterAttributeName", TokenizerState::AfterAttributeName},
      {"BeforeAttributeValue", TokenizerState::BeforeAttributeValue},
      {"AttributeValueDoubleQuoted", TokenizerState::AttributeValueDoubleQuoted},
      {"AttributeValueSingleQuoted", TokenizerState::AttributeValueSingleQuoted},
      {"AttributeValueUnquoted", TokenizerState::AttributeValueUnquoted},
      {"AfterAttributeValueQuoted", TokenizerState::AfterAttributeValueQuoted},
      {"SelfClosingStartTag", TokenizerState::SelfClosingStartTag},
      {"BogusComment", TokenizerState::BogusComment},
      {"MarkupDeclarationOpen", TokenizerState::MarkupDeclarationOpen},
      {"CommentStart", TokenizerState::CommentStart},
      {"CommentStartDash", TokenizerState::CommentStartDash},
      {"Comment", TokenizerState::Comment},
      {"CommentLessThanSign", TokenizerState::CommentLessThanSign},
      {"CommentLessThanSignBang", TokenizerState::CommentLessThanSignBang},
      {"CommentLessThanSignBangDash", TokenizerState::CommentLessThanSignBangDash},
      {"CommentLessThanSignBangDashDash", TokenizerState::CommentLessThanSignBangDashDash},
      {"CommentEndDash", TokenizerState::CommentEndDash},
      {"CommentEnd", TokenizerState::CommentEnd},
      {"CommentEndBang", TokenizerState::CommentEndBang},
      {"DOCTYPE", TokenizerState::DOCTYPE},
      {"BeforeDOCTYPEName", TokenizerState::BeforeDOCTYPEName},
      {"DOCTYPEName", TokenizerState::DOCTYPEName},
      {"AfterDOCTYPEName", TokenizerState::AfterDOCTYPEName},
      {"AfterDOCTYPEPublicKeyword", TokenizerState::AfterDOCTYPEPublicKeyword},
      {"BeforeDOCTYPEPublicIdentifier", TokenizerState::BeforeDOCTYPEPublicIdentifier},
      {"DOCTYPEPublicIdentifierDoubleQuoted", TokenizerState::DOCTYPEPublicIdentifierDoubleQuoted},
      {"DOCTYPEPublicIdentifierSingleQuoted", TokenizerState::DOCTYPEPublicIdentifierSingleQuoted},
      {"AfterDOCTYPEPublicIdentifier", TokenizerState::AfterDOCTYPEPublicIdentifier},
      {"BetweenDOCTYPEPublicAndSystemIdentifiers", TokenizerState::BetweenDOCTYPEPublicAndSystemIdentifiers},
      {"AfterDOCTYPESystemKeyword", TokenizerState::AfterDOCTYPESystemKeyword},
      {"BeforeDOCTYPESystemIdentifier", TokenizerState::BeforeDOCTYPESystemIdentifier},
      {"DOCTYPESystemIdentifierDoubleQuoted", TokenizerState::DOCTYPESystemIdentifierDoubleQuoted},
      {"DOCTYPESystemIdentifierSingleQuoted", TokenizerState::DOCTYPESystemIdentifierSingleQuoted},
      {"AfterDOCTYPESystemIdentifier", TokenizerState::AfterDOCTYPESystemIdentifier},
      {"BogusDOCTYPE", TokenizerState::BogusDOCTYPE},
      {"CDATASection", TokenizerState::CDATASection},
      {"CDATASectionBracket", TokenizerState::CDATASectionBracket},
      {"CDATASectionEnd", TokenizerState::CDATASectionEnd},
      {"ProcessingInstructionOpen", TokenizerState::ProcessingInstructionOpen},
      {"ProcessingInstructionTarget", TokenizerState::ProcessingInstructionTarget},
      {"AfterProcessingInstructionTarget", TokenizerState::AfterProcessingInstructionTarget},
      {"ProcessingInstructionData", TokenizerState::ProcessingInstructionData},
      {"ProcessingInstructionQuestionable", TokenizerState::ProcessingInstructionQuestionable},
      {"CharacterReference", TokenizerState::CharacterReference},
      {"NamedCharacterReference", TokenizerState::NamedCharacterReference},
      {"AmbiguousAmpersand", TokenizerState::AmbiguousAmpersand},
      {"NumericCharacterReference", TokenizerState::NumericCharacterReference},
      {"HexadecimalCharacterReferenceStart", TokenizerState::HexadecimalCharacterReferenceStart},
      {"DecimalCharacterReferenceStart", TokenizerState::DecimalCharacterReferenceStart},
      {"HexadecimalCharacterReference", TokenizerState::HexadecimalCharacterReference},
      {"DecimalCharacterReference", TokenizerState::DecimalCharacterReference},
      {"NumericCharacterReferenceEnd", TokenizerState::NumericCharacterReferenceEnd},
    };

    constexpr std::pair<stringview, ParseError> TokenizerTestErrorNames[] = {
      {"UnexpectedNullCharacter", ParseError::UnexpectedNullCharacter},
      {"EOFBeforeTagName", ParseError::EOFBeforeTagName},
      {"InvalidFirstCharacterOfTagName", ParseError::InvalidFirstCharacterOfTagName},
      {"MissingEndTagName", ParseError::MissingEndTagName},
      {"EOFInTag", ParseError::EOFInTag},
      {"EOFInScriptHTMLCommentLikeText", ParseError::EOFInScriptHTMLCommentLikeText},
      {"UnexpectedEqualsSignBeforeAttributeName", ParseError::UnexpectedEqualsSignBeforeAttributeName},
      {"UnexpectedCharacterInAttributeName", ParseError::UnexpectedCharacterInAttributeName},
      {"DuplicateAttribute", ParseError::DuplicateAttribute},
      {"MissingAttributeValue", ParseError::MissingAttributeValue},
      {"UnexpectedCharacterInUnquotedAttributeValue",
       ParseError::UnexpectedCharacterInUnquotedAttributeValue},
      {"MissingWhitespaceBetweenAttributes", ParseError::MissingWhitespaceBetweenAttributes},
      {"UnexpectedSolidusInTag", ParseError::UnexpectedSolidusInTag},
      {"CDATAInHTMLContent", ParseError::CDATAInHTMLContent},
      {"IncorrectlyOpenedComment", ParseError::IncorrectlyOpenedComment},
      {"AbruptClosingOfEmptyComment", ParseError::AbruptClosingOfEmptyComment},
      {"EOFInComment", ParseError::EOFInComment},
      {"NestedComment", ParseError::NestedComment},
      {"IncorrectlyClosedComment", ParseError::IncorrectlyClosedComment},
      {"EOFInDOCTYPE", ParseError::EOFInDOCTYPE},
      {"MissingWhitespaceBeforeDOCTYPEName", ParseError::MissingWhitespaceBeforeDOCTYPEName},
      {"InvalidCharacterSequenceAfterDOCTYPEName", ParseError::InvalidCharacterSequenceAfterDOCTYPEName},
      {"MissingDOCTYPEName", ParseError::MissingDOCTYPEName},
      {"MissingWhitespaceAfterDOCTYPEPublicKeyword", ParseError::MissingWhitespaceAfterDOCTYPEPublicKeyword},
      {"MissingDOCTYPEPublicIdentifier", ParseError::MissingDOCTYPEPublicIdentifier},
      {"MissingQuoteBeforeDOCTYPEPublicIdentifier", ParseError::MissingQuoteBeforeDOCTYPEPublicIdentifier},
      {"AbruptDOCTYPEPublicIdentifier", ParseError::AbruptDOCTYPEPublicIdentifier},
      {"MissingWhitespaceBetweenDOCTYPEPublicAndSystemIdentifiers",
       ParseError::MissingWhitespaceBetweenDOCTYPEPublicAndSystemIdentifiers},
      {"MissingQuoteBeforeDOCTYPESystemIdentifier", ParseError::MissingQuoteBeforeDOCTYPESystemIdentifier},
      {"MissingWhitespaceAfterDOCTYPESystemKeyword", ParseError::MissingWhitespaceAfterDOCTYPESystemKeyword},
      {"MissingDOCTYPESystemIdentifier", ParseError::MissingDOCTYPESystemIdentifier},
      {"AbruptDOCTYPESystemIdentifier", ParseError::AbruptDOCTYPESystemIdentifier},
      {"UnexpectedCharacterAfterDOCTYPESystemIdentifier",
       ParseError::UnexpectedCharacterAfterDOCTYPESystemIdentifier},
      {"EOFInCDATA", ParseError::EOFInCDATA},
      {"EOFInProcessingInstruction", ParseError::EOFInProcessingInstruction},
      {"InvalidFirstCharacterOfProcessingInstructionTarget",
       ParseError::InvalidFirstCharacterOfProcessingInstructionTarget},
      {"InvalidProcessingInstructionTarget", ParseError::InvalidProcessingInstructionTarget},
      {"DisallowedProcessingInstructionTarget", ParseError::DisallowedProcessingInstructionTarget},
      {"UnknownNamedCharacterReference", ParseError::UnknownNamedCharacterReference},
      {"AbsenceOfDigitsInNumericCharacterReference", ParseError::AbsenceOfDigitsInNumericCharacterReference},
      {"MissingSemicolonAfterCharacterReference", ParseError::MissingSemicolonAfterCharacterReference},
      {"NullCharacterReference", ParseError::NullCharacterReference},
      {"CharacterReferenceOutsideUnicodeRange", ParseError::CharacterReferenceOutsideUnicodeRange},
      {"SurrogateCharacterReference", ParseError::SurrogateCharacterReference},
      {"NonCharacterCharacterReference", ParseError::NonCharacterCharacterReference},
      {"ControlCharacterReference", ParseError::ControlCharacterReference},
    };

    template <typename E, size_t N>
    consteval bool TokenizerTestNamesInEnumOrder(const std::pair<stringview, E> (&names)[N]) noexcept
    {
      for (size_t i = 0; i < N; ++i)
      {
        if (ToUnderlying(names[i].second) != i)
        {
          return false;
        }
      }
      return N == ToUnderlying(MaxEnumValue(E {})) + 1uz;
    }

    static_assert(TokenizerTestNamesInEnumOrder(TokenizerTestStateNames));
    static_assert(TokenizerTestNamesInEnumOrder(TokenizerTestErrorNames));

    template <typename E, size_t N>
    KRYS_NODISCARD Maybe<E> TokenizerTestEnumFromName(const std::pair<stringview, E> (&names)[N],
                                                      stringview name) noexcept
    {
      auto lower = [](char c)
      {
        return c >= 'A' && c <= 'Z' ? static_cast<char>(c - 'A' + 'a') : c;
      };
      for (const auto &[candidate, value] : names)
      {
        if (std::ranges::equal(candidate, name, {}, lower, lower))
        {
          return value;
        }
      }
      return {};
    }

    void SerialiseHTMLToken(Token &token, DOMString &output) noexcept
    {
      switch (token.Type())
      {
        case TokenType::DOCTYPE:
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
        case TokenType::Character:
        {
          output += u8"Character(";
          output += DOMStringView(token.Data());
          output += u8")\n";
          break;
        }
        case TokenType::Comment:
        {
          output += u8"Comment(";
          output += DOMStringView(token.Data());
          output += u8")\n";
          break;
        }
        case TokenType::ProcessingInstruction:
        {
          output += u8"ProcessingInstruction(";
          output += DOMStringView(token.Data());
          output += u8", ";
          output += DOMStringView(token.ProcessingInstructionData());
          output += u8")\n";
          break;
        }
        case TokenType::StartTag:
        case TokenType::EndTag:
        {
          if (token.Type() == TokenType::StartTag)
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

          if (token.Type() == TokenType::StartTag)
          {
            output += u8", SelfClosing=";
            output += token.IsSelfClosing() ? u8"true" : u8"false";
          }

          output += u8")\n";
          break;
        }
        case TokenType::EndOfFile:
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
      List<TokenizerError> Errors;
    };

    KRYS_NODISCARD List<HTMLTokenizerTest> ParseHTMLTokenizerTests(std::istream &stream) noexcept
    {
      List<HTMLTokenizerTest> tests;
      HTMLTokenizerTest currentTest;

      auto parse = [&](const utf8_string &sectionName, utf8_string data)
      {
        if (sectionName == u8"name")
        {
          if (!currentTest.Name.empty() && !currentTest.Html.empty())
          {
            tests.push_back(krys::move(currentTest));
            currentTest = {};
          }

          currentTest.Name = string(data.begin(), data.end());
        }
        else if (sectionName == u8"data")
        {
          krys::boo::tests::NormaliseData(data);
          currentTest.Html = krys::move(data);
        }
        else if (sectionName == u8"tokens")
        {
          krys::boo::tests::NormaliseData(data);
          currentTest.Tokens = krys::move(data);
        }
        else if (sectionName == u8"initial-state")
        {
          krys::boo::tests::NormaliseData(data);

          auto state = TokenizerTestEnumFromName(
            TokenizerTestStateNames, stringview(reinterpret_cast<const char *>(data.data()), data.size()));
          if (state.has_value())
          {
            currentTest.InitialState = *state;
          }
        }
        else if (sectionName == u8"expected-state")
        {
          krys::boo::tests::NormaliseData(data);

          auto state = TokenizerTestEnumFromName(
            TokenizerTestStateNames, stringview(reinterpret_cast<const char *>(data.data()), data.size()));
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
          auto lines = krys::boo::tests::SplitStringByNewline(data);

          for (const auto &line : lines)
          {
            auto text = stringview(line);
            // NOTE: errors are in the form "line:column ErrorName"

            uint32 lineNo = 0u;
            auto [ptr1, ec1] = std::from_chars(text.data(), text.data() + text.size(), lineNo);
            text.remove_prefix(ptr1 - text.data());

            text.remove_prefix(1uz); // skip ':'

            uint32 column = 0u;
            auto [ptr2, ec2] = std::from_chars(text.data(), text.data() + text.size(), column);
            text.remove_prefix(ptr2 - text.data());

            text.remove_prefix(1uz); // skip ' '

            auto error = TokenizerTestEnumFromName(TokenizerTestErrorNames, text);
            if (error.has_value())
            {
              currentTest.Errors.push_back({
                .Error = *error,
                .Location =
                  {
                    .Line = lineNo,
                    .Column = column,
                  },
              });
            }
          }
        }
      };

      krys::boo::tests::ParseTestData(stream, "#", ::krys::move(parse));
      if (!currentTest.Html.empty())
      {
        tests.push_back(krys::move(currentTest));
      }

      return tests;
    }

    void ExecuteHTMLTokenizerTest(const HTMLTokenizerTest &test, size_t number, size_t total) noexcept
    {
      InputStream inputStream;
      Tokenizer tokenizer(inputStream, [allowed = test.CDATASectionAllowed]() { return allowed; });
      tokenizer.State(test.InitialState);
      if (test.AppendEOF)
      {
        inputStream.Append(test.Html, IsEOF::Yes);
      }
      else
      {
        inputStream.Append(test.Html, IsEOF::No);
      }

      DOMString tokens;
      while (true)
      {
        auto token = tokenizer.NextToken();
        if (!token)
        {
          break;
        }
        SerialiseHTMLToken(*token, tokens);
      }

      krys::boo::tests::NormaliseData(tokens);

      utf8_string str = u8"--- TEST " + krys::boo::tests::ToUTF8(number + 1uz) + u8" OF "
                        + krys::boo::tests::ToUTF8(total) + u8" ---\n"
                        + krys::boo::tests::ToUTF8(test.Name.empty() ? "Unnamed Test" : test.Name);
      KRYS_UTF8_INFO(str);

      str = u8"\n--- INPUT ---\n" + test.Html;
      KRYS_UTF8_INFO(str);

      str = u8"--- EXPECTED TOKENS ---\n" + test.Tokens;
      KRYS_UTF8_INFO(str);

      str = u8"\n--- ACTUAL TOKENS ---\n" + tokens;
      KRYS_UTF8_INFO(str);

      auto initialState = string(TokenizerTestStateNames[ToUnderlying(test.InitialState)].first);
      str = u8"\n--- INITIAL STATE ---\n" + krys::boo::tests::ToUTF8(initialState);

      auto expectedState = string(TokenizerTestStateNames[ToUnderlying(test.ExpectedState)].first);
      str = u8"\n--- EXPECTED STATE ---\n" + krys::boo::tests::ToUTF8(expectedState);

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

    void RunTokenizerTest(const string &filename) noexcept
    {
      static string basedir = KRYS_BOOEY_TESTS_DATA_DIR "/html-tokenizer/";

      auto file = krys::boo::tests::OpenTestDataFile(basedir + filename);
      REQUIRE(file.has_value());

      auto tests = ParseHTMLTokenizerTests(*file);
      REQUIRE(!tests.empty());

      krys::boo::tests::ExecuteTests(tests, ExecuteHTMLTokenizerTest);
    }
  }

#define KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE(FileName, Tags)                                                \
  TEST_CASE("Tokenizer(" FileName ")", "[HTML][Tokenizer]" Tags)                                             \
  {                                                                                                          \
    RunTokenizerTest(FileName);                                                                              \
  }

  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("afterattributename-01.dat", "[html:after-attribute-name-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("afterattributevaluequoted-01.dat",
                                        "[html:after-attribute-value-(quoted)-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("afterdoctypename-01.dat", "[html:after-doctype-name-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("afterdoctypepublicidentifier-01.dat",
                                        "[html:after-doctype-public-identifier-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("afterdoctypepublickeyword-01.dat",
                                        "[html:after-doctype-public-keyword-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("afterdoctypesystemidentifier-01.dat",
                                        "[html:after-doctype-system-identifier-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("afterdoctypesystemkeyword-01.dat",
                                        "[html:after-doctype-system-keyword-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("attributename-01.dat", "[html:attribute-name-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("attributevaluedoublequoted-01.dat",
                                        "[html:attribute-value-(double-quoted)-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("attributevaluesinglequoted-01.dat",
                                        "[html:attribute-value-(single-quoted)-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("attributevalueunquoted-01.dat",
                                        "[html:attribute-value-(unquoted)-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("beforeattributename-01.dat", "[html:before-attribute-name-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("beforeattributevalue-01.dat", "[html:before-attribute-value-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("beforedoctypename-01.dat", "[html:before-doctype-name-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("beforedoctypepublicidentifier-01.dat",
                                        "[html:before-doctype-public-identifier-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("beforedoctypesystemidentifier-01.dat",
                                        "[html:before-doctype-system-identifier-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("betweendoctypepublicandsystemidentifiers-01.dat",
                                        "[html:between-doctype-public-and-system-identifiers-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("boguscomment-01.dat", "[html:bogus-comment-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("bogusdoctype-01.dat", "[html:bogus-doctype-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("cdatasection-01.dat", "[html:cdata-section-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("cdatasectionbracket-01.dat", "[html:cdata-section-bracket-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("cdatasectionend-01.dat", "[html:cdata-section-end-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE(
    "character-reference-01.dat",
    "[html:character-reference-state][html:named-character-reference-state][html:numeric-character-reference-"
    "state][html:decimal-character-reference-state][html:hexadecimal-character-reference-state][html:numeric-"
    "character-reference-end-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE(
    "character-reference-02.dat",
    "[html:character-reference-state][html:named-character-reference-state][html:ambiguous-ampersand-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE(
    "character-reference-03.dat",
    "[html:character-reference-state][html:numeric-character-reference-state][html:decimal-character-"
    "reference-state][html:numeric-character-reference-end-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE(
    "character-reference-04.dat", "[html:character-reference-state][html:numeric-character-reference-state]["
                                  "html:hexadecimal-character-reference-start-state][html:hexadecimal-"
                                  "character-reference-state][html:numeric-character-reference-end-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("comment-01.dat", "[html:comment-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("commentend-01.dat", "[html:comment-end-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("commentendbang-01.dat", "[html:comment-end-bang-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("commentenddash-01.dat", "[html:comment-end-dash-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("commentlessthan-01.dat", "[html:comment-less-than-sign-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("commentlessthanbang-01.dat",
                                        "[html:comment-less-than-sign-bang-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("commentlessthanbangdash-01.dat",
                                        "[html:comment-less-than-sign-bang-dash-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("commentlessthanbangdashdash-01.dat",
                                        "[html:comment-less-than-sign-bang-dash-dash-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("commentstart-01.dat", "[html:comment-start-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("commentstartdash-01.dat", "[html:comment-start-dash-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("data-01.dat", "[html:data-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("doctype-01.dat", "[html:doctype-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("doctypename-01.dat", "[html:doctype-name-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("doctypepublicidentifierdoublequoted-01.dat",
                                        "[html:doctype-public-identifier-(double-quoted)-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("doctypepublicidentifiersinglequoted-01.dat",
                                        "[html:doctype-public-identifier-(single-quoted)-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("doctypesystemidentifierdoublequoted-01.dat",
                                        "[html:doctype-system-identifier-(double-quoted)-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("doctypesystemidentifiersinglequoted-01.dat",
                                        "[html:doctype-system-identifier-(single-quoted)-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("endtagopen-01.dat", "[html:end-tag-open-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("markupdeclarationopen-01.dat",
                                        "[html:markup-declaration-open-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE(
    "processinginstruction-01.dat",
    "[html:processing-instruction-open-state][html:processing-instruction-target-state][html:after-"
    "processing-"
    "instruction-target-state][html:processing-instruction-data-state][html:processing-instruction-"
    "questionable-state][html:convert-the-temporary-buffer-to-a-comment]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("plaintext-01.dat", "[html:plaintext-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("rawtext-01.dat", "[html:rawtext-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("rawtextendtagopen-01.dat", "[html:rawtext-end-tag-open-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("rawtextlessthan-01.dat", "[html:rawtext-less-than-sign-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("rcdata-01.dat", "[html:rcdata-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("rcdataendtagname-01.dat", "[html:rcdata-end-tag-name-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("rcdataendtagopen-01.dat", "[html:rcdata-end-tag-open-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("rcdatalessthan-01.dat", "[html:rcdata-less-than-sign-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdata-01.dat", "[html:script-data-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescaped-01.dat",
                                        "[html:script-data-double-escaped-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescapeddash-01.dat",
                                        "[html:script-data-double-escaped-dash-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescapeddashdash-01.dat",
                                        "[html:script-data-double-escaped-dash-dash-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescapedlessthan-01.dat",
                                        "[html:script-data-double-escaped-less-than-sign-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescapeend-01.dat",
                                        "[html:script-data-double-escape-end-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescapestart-01.dat",
                                        "[html:script-data-double-escape-start-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataendtagname-01.dat",
                                        "[html:script-data-end-tag-name-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataendtagopen-01.dat",
                                        "[html:script-data-end-tag-open-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataescaped-01.dat", "[html:script-data-escaped-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataescapeddash-01.dat",
                                        "[html:script-data-escaped-dash-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataescapeddashdash-01.dat",
                                        "[html:script-data-escaped-dash-dash-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataescapedendtagname-01.dat",
                                        "[html:script-data-escaped-end-tag-name-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataescapedendtagopen-01.dat",
                                        "[html:script-data-escaped-end-tag-open-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataescapedlessthan-01.dat",
                                        "[html:script-data-escaped-less-than-sign-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdataescapestart-01.dat",
                                        "[html:script-data-escape-start-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("scriptdatalessthan-01.dat",
                                        "[html:script-data-less-than-sign-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("selfclosingstartag-01.dat", "[html:self-closing-start-tag-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("tagname-01.dat", "[html:tag-name-state]");
  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("tagopen-01.dat", "[html:tag-open-state]");

  KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE("integration-test-01.dat", "[html:tokenization]");
}

#undef KRYS_EXECUTE_HTML_TOKENIZER_TEST_CASE
