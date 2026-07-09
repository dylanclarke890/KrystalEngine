#include "Krystal.HTML.Tests/ParserTestUtils.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenizer.hpp"
#include <catch_all.hpp>
#include <format>

namespace Krys::Tests
{
  using namespace Krys::HTML;

#define SETUP_TEST(initialState)                                                                             \
  HTMLInputStream inputStream;                                                                               \
  HTMLTokenizer tokenizer(inputStream, []() { return false; });                                              \
  tokenizer.State(initialState);                                                                             \
  const auto &errors = tokenizer.ParseErrors();                                                              \
  size_t expectedErrorCount = 0;                                                                             \
  utf8_string expected = u8"";

#define COMMON_TEST_CASES(tokenType)                                                                         \
  REQUIRE(token);                                                                                            \
  REQUIRE(token->Type() == tokenType);                                                                       \
  REQUIRE(Compare(token->Data(), expected));                                                                 \
  REQUIRE(errors.size() == expectedErrorCount);

#pragma region CharacterReference

  TEST("CharacterReference", "Non-character reference",
       (UnitTest {.Input = U"&_", .Output = {CreateCharacterToken(u8"&_")}}))

#pragma region NamedCharacterReference

  TEST("NamedCharacterReference", "Basic named character reference",
       (UnitTest {.Input = U"&copy;", .Output = {CreateCharacterToken(u8"©")}}))

  TEST("NamedCharacterReference", "missing semicolon",
       (UnitTest {
         .Input = U"&Agrave",
         .AppendEOF = true,
         .Output = {CreateCharacterToken(u8"À"), CreateEOFToken()},
         .Errors = {
           {.Error = HTMLParseError::MissingSemicolonAfterCharacterReference, .Line = 1uz, .Column = 8uz}}}))

  TEST("NamedCharacterReference", "no match, incomplete reference",
       (UnitTest {.ExpectedState = HTML::TokenizerState::AmbiguousAmpersand,
                  .Input = U"&nonentity",
                  .Output = {CreateCharacterToken(u8"&nonentity")}}))

  TEST("NamedCharacterReference", "no match, ends in semicolon",
       (UnitTest {
         .Input = U"&nonentity;",
         .Output = {CreateCharacterToken(u8"&nonentity;")},
         .Errors = {{.Error = HTMLParseError::UnknownNamedCharacterReference, .Line = 1uz, .Column = 11uz}}}))

  TEST("NamedCharacterReference", "mixed case reference",
       (UnitTest {.Input = U"&vsupnE;", .Output = {CreateCharacterToken(u8"⫌︀")}}))

  TEST("NamedCharacterReference", "partial match with valid prefix",
       (UnitTest {
         .Input = U"&notit;",
         .Output = {CreateCharacterToken(u8"¬it;")},
         .Errors = {
           {.Error = HTMLParseError::MissingSemicolonAfterCharacterReference, .Line = 1uz, .Column = 6uz}}}))

  TEST("NamedCharacterReference", "EOF in middle of otherwise valid reference",
       (UnitTest {
         .Input = U"&co", .AppendEOF = true, .Output = {CreateCharacterToken(u8"&co"), CreateEOFToken()}}))

  // TODO(test): test cases for when character references are consumed as part of attributes

#pragma endregion

#pragma region DecimalCharacterReference

  TEST("DecimalCharacterReference", "happy path",
       (UnitTest {.Input = U"&#8482;", .Output = {CreateCharacterToken(u8"™")}}))

  TEST("DecimalCharacterReference", "lookup table",
       (UnitTest {.Input = U"&#128;", .Output = {CreateCharacterToken(u8"€")}}))

  TEST("DecimalCharacterReference", "leading zeros",
       (UnitTest {.Input = U"&#00008482;", .Output = {CreateCharacterToken(u8"™")}}))

  TEST("DecimalCharacterReference", "missing semicolon",
       (UnitTest {
         .Input = U"&#8482",
         .AppendEOF = true,
         .Output = {CreateCharacterToken(u8"™"), CreateEOFToken()},
         .Errors = {
           {.Error = HTMLParseError::MissingSemicolonAfterCharacterReference, .Line = 1uz, .Column = 7uz}}}))

  TEST("DecimalCharacterReference", "no numbers provided after #",
       (UnitTest {.Input = U"&#;",
                  .Output = {CreateCharacterToken(u8"&#;")},
                  .Errors = {{.Error = HTMLParseError::AbsenceOfDigitsInNumericCharacterReference,
                              .Line = 1uz,
                              .Column = 3uz}}}))

  TEST("DecimalCharacterReference", "mixed with non-digit characters",
       (UnitTest {
         .Input = U"&#123abc;",
         .Output = {CreateCharacterToken(u8"{abc;")},
         .Errors = {
           {.Error = HTMLParseError::MissingSemicolonAfterCharacterReference, .Line = 1uz, .Column = 6uz}}}))

  TEST("DecimalCharacterReference", "stops at semicolon",
       (UnitTest {.Input = U"&#65;BC", .Output = {CreateCharacterToken(u8"ABC")}}))

  TEST("DecimalCharacterReference", "stops at non-digit character",
       (UnitTest {
         .Input = U"&#65BC;",
         .Output = {CreateCharacterToken(u8"ABC;")},
         .Errors = {
           {.Error = HTMLParseError::MissingSemicolonAfterCharacterReference, .Line = 1uz, .Column = 5uz}}}))

  TEST("DecimalCharacterReference", "null character reference",
       (UnitTest {.Input = U"&#0;",
                  .Output = {CreateCharacterToken(u8"\uFFFD")},
                  .Errors = {{.Error = HTMLParseError::NullCharacterReference, .Line = 1uz, .Column = 4uz}}}))

  TEST("DecimalCharacterReference", "character reference outside unicode range",
       (UnitTest {
         .Input = U"&#1114112;",
         .Output = {CreateCharacterToken(u8"\uFFFD")},
         .Errors = {
           {.Error = HTMLParseError::CharacterReferenceOutsideUnicodeRange, .Line = 1uz, .Column = 10uz}}}))

  TEST("DecimalCharacterReference", "surrogate",
       (UnitTest {
         .Input = U"&#55296;",
         .Output = {CreateCharacterToken(u8"\uFFFD")},
         .Errors = {{.Error = HTMLParseError::SurrogateCharacterReference, .Line = 1uz, .Column = 8uz}}}))

  TEST("DecimalCharacterReference", "non character",
       (UnitTest {
         .Input = U"&#65534;",
         .Output = {CreateCharacterToken(u8"\uFFFE")},
         .Errors = {{.Error = HTMLParseError::NonCharacterCharacterReference, .Line = 1uz, .Column = 8uz}}}))

  TEST("DecimalCharacterReference", "control character",
       (UnitTest {
         .Input = U"&#13;",
         .Output = {CreateCharacterToken(u8"\x0D")},
         .Errors = {{.Error = HTMLParseError::ControlCharacterReference, .Line = 1uz, .Column = 5uz}}}))

#pragma endregion

#pragma region HexadecimalCharacterReference

  TEST("HexadecimalCharacterReference", "happy path",
       (UnitTest {.Input = U"&#x152;", .Output = {CreateCharacterToken(u8"Œ")}}))

  TEST("HexadecimalCharacterReference", "lookup table",
       (UnitTest {.Input = U"&#x80;", .Output = {CreateCharacterToken(u8"€")}}))

  TEST("HexadecimalCharacterReference", "mixed case hex digits",
       (UnitTest {.Input = U"&#x20Ac;", .Output = {CreateCharacterToken(u8"€")}}))

  TEST("HexadecimalCharacterReference", "uppercase X",
       (UnitTest {.Input = U"&#X152;", .Output = {CreateCharacterToken(u8"Œ")}}))

  TEST("HexadecimalCharacterReference", "missing semicolon",
       (UnitTest {
         .Input = U"&#X152",
         .AppendEOF = true,
         .Output = {CreateCharacterToken(u8"Œ"), CreateEOFToken()},
         .Errors = {
           {.Error = HTMLParseError::MissingSemicolonAfterCharacterReference, .Line = 1uz, .Column = 7uz}}}))

  TEST("HexadecimalCharacterReference", "no numbers provided after #X",
       (UnitTest {.Input = U"&#X;",
                  .Output = {CreateCharacterToken(u8"&#X;")},
                  .Errors = {{.Error = HTMLParseError::AbsenceOfDigitsInNumericCharacterReference,
                              .Line = 1uz,
                              .Column = 4uz}}}))

  TEST("HexadecimalCharacterReference", "null character reference",
       (UnitTest {.Input = U"&#x00;",
                  .Output = {CreateCharacterToken(u8"\uFFFD")},
                  .Errors = {{.Error = HTMLParseError::NullCharacterReference, .Line = 1uz, .Column = 6uz}}}))

  TEST("HexadecimalCharacterReference", "character reference outside unicode range",
       (UnitTest {
         .Input = U"&#x110000;",
         .Output = {CreateCharacterToken(u8"\uFFFD")},
         .Errors = {
           {.Error = HTMLParseError::CharacterReferenceOutsideUnicodeRange, .Line = 1uz, .Column = 10uz}}}))

  TEST("HexadecimalCharacterReference", "surrogate",
       (UnitTest {
         .Input = U"&#xD800;",
         .Output = {CreateCharacterToken(u8"\uFFFD")},
         .Errors = {{.Error = HTMLParseError::SurrogateCharacterReference, .Line = 1uz, .Column = 8uz}}}))

  TEST("HexadecimalCharacterReference", "non character",
       (UnitTest {
         .Input = U"&#xFFFE;",
         .Output = {CreateCharacterToken(u8"\uFFFE")},
         .Errors = {{.Error = HTMLParseError::NonCharacterCharacterReference, .Line = 1uz, .Column = 8uz}}}))

  TEST("HexadecimalCharacterReference", "control character",
       (UnitTest {
         .Input = U"&#x0D;",
         .Output = {CreateCharacterToken(u8"\x0D")},
         .Errors = {{.Error = HTMLParseError::ControlCharacterReference, .Line = 1uz, .Column = 6uz}}}))

#pragma endregion

  TEST("CharacterReference", "Multiple character references",
       (UnitTest {
         .Input = U"&copy;&Agrave&#128;&#X152;&a;",
         .Output = {CreateCharacterToken(u8"©À€Œ&a;")},
         .Errors = {
           {.Error = HTMLParseError::MissingSemicolonAfterCharacterReference, .Line = 1uz, .Column = 14uz},
           {.Error = HTMLParseError::UnknownNamedCharacterReference, .Line = 1uz, .Column = 29uz}}}))

#pragma endregion

#pragma region Data

  TEST("Data", "Batches characters ",
       (UnitTest {.Input = U"a string of characters; 123145",
                  .Output = {CreateCharacterToken(u8"a string of characters; 123145")}}))

  TEST("Data", "Replaces character references",
       (UnitTest {.Input = U"Some data &copy; some more data",
                  .Output = {CreateCharacterToken(u8"Some data © some more data")}}))

  TEST("Data", "switches to TagOpen when parsing LessThanSign",
       (UnitTest {.ExpectedState = TokenizerState::TagOpen, .Input = U"<", .Output = {}}))

  TEST("Data", "Batches characters up to less than sign",
       (UnitTest {.Input = U"a string of characters; 123145<",
                  .Output = {CreateCharacterToken(u8"a string of characters; 123145")}}))

  TEST("Data", "Batches characters up to EOF then emits EOF",
       (UnitTest {.Input = U"a string of characters; 123145",
                  .AppendEOF = true,
                  .Output = {CreateCharacterToken(u8"a string of characters; 123145"), CreateEOFToken()}}))

  TEST("Data", "Emits null character as-is with parse error",
       (UnitTest {
         .Input = InsertUTF32Null(U"1234"),
         .Output = {CreateCharacterToken(InsertUTF8Null(u8"1234"))},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 5uz}}}))

#pragma endregion

#pragma region RCDATA

  TEST("RCDATA", "Batches characters",
       (UnitTest {.InitialState = TokenizerState::RCDATA,
                  .ExpectedState = TokenizerState::RCDATA,
                  .Input = U"a string of characters; 123145",
                  .Output = {CreateCharacterToken(u8"a string of characters; 123145")}}))

  TEST("RCDATA", "Replaces character references",
       (UnitTest {.InitialState = TokenizerState::RCDATA,
                  .ExpectedState = TokenizerState::RCDATA,
                  .Input = U"Some data &copy; some more data",
                  .Output = {CreateCharacterToken(u8"Some data © some more data")}}))

  TEST("RCDATA", "switches to RCDATALessThanSign when parsing LessThanSign",
       (UnitTest {.InitialState = TokenizerState::RCDATA,
                  .ExpectedState = TokenizerState::RCDATALessThanSign,
                  .Input = U"<",
                  .Output = {}}))

  TEST("RCDATA", "Emits null character as U+FFFD with parse error",
       (UnitTest {
         .InitialState = TokenizerState::RCDATA,
         .ExpectedState = TokenizerState::RCDATA,
         .Input = InsertUTF32Null(U"1234"),
         .Output = {CreateCharacterToken(u8"1234\uFFFD")},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 5uz}}}))

  TEST("RCDATA", "Batches characters up to EOF then emits EOF",
       (UnitTest {.InitialState = TokenizerState::RCDATA,
                  .Input = U"a string of characters; 123145",
                  .AppendEOF = true,
                  .Output = {CreateCharacterToken(u8"a string of characters; 123145"), CreateEOFToken()}}))

#pragma endregion

#pragma region RAWTEXT

  TEST("RAWTEXT", "switches to RAWTEXTLessThanSign when parsing LessThanSign",
       (UnitTest {.InitialState = TokenizerState::RAWTEXT,
                  .ExpectedState = TokenizerState::RAWTEXTLessThanSign,
                  .Input = U"<",
                  .Output = {}}))

  TEST("RAWTEXT", "Emits null character as U+FFFD with parse error",
       (UnitTest {
         .InitialState = TokenizerState::RAWTEXT,
         .ExpectedState = TokenizerState::RAWTEXT,
         .Input = InsertUTF32Null(U"1234"),
         .Output = {CreateCharacterToken(u8"1234\uFFFD")},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 5uz}}}))

  TEST("RAWTEXT", "Batches characters up to EOF then emits EOF",
       (UnitTest {.InitialState = TokenizerState::RAWTEXT,
                  .Input = U"a string of characters; 123145",
                  .AppendEOF = true,
                  .Output = {CreateCharacterToken(u8"a string of characters; 123145"), CreateEOFToken()}}))

  TEST("RAWTEXT", "Batches characters",
       (UnitTest {.InitialState = TokenizerState::RAWTEXT,
                  .ExpectedState = TokenizerState::RAWTEXT,
                  .Input = U"a string of characters; 123145",
                  .Output = {CreateCharacterToken(u8"a string of characters; 123145")}}))

#pragma endregion

#pragma region ScriptData

  TEST("ScriptData", "switches to ScriptDataLessThanSign when parsing LessThanSign",
       (UnitTest {.InitialState = TokenizerState::ScriptData,
                  .ExpectedState = TokenizerState::ScriptDataLessThanSign,
                  .Input = U"<",
                  .Output = {}}))

  TEST("ScriptData", "Emits null character as U+FFFD with parse error",
       (UnitTest {
         .InitialState = TokenizerState::ScriptData,
         .ExpectedState = TokenizerState::ScriptData,
         .Input = InsertUTF32Null(U"1234"),
         .Output = {CreateCharacterToken(u8"1234\uFFFD")},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 5uz}}}))

  TEST("ScriptData", "Batches characters up to EOF then emits EOF",
       (UnitTest {.InitialState = TokenizerState::ScriptData,
                  .Input = U"a string of characters; 123145",
                  .AppendEOF = true,
                  .Output = {CreateCharacterToken(u8"a string of characters; 123145"), CreateEOFToken()}}))

  TEST("ScriptData", "Batches characters",
       (UnitTest {.InitialState = TokenizerState::ScriptData,
                  .ExpectedState = TokenizerState::ScriptData,
                  .Input = U"a string of characters; 123145",
                  .Output = {CreateCharacterToken(u8"a string of characters; 123145")}}))

#pragma endregion

#pragma region PLAINTEXT

  TEST("PLAINTEXT", "Emits null character as U+FFFD with parse error",
       (UnitTest {
         .InitialState = TokenizerState::PLAINTEXT,
         .ExpectedState = TokenizerState::PLAINTEXT,
         .Input = InsertUTF32Null(U"1234"),
         .Output = {CreateCharacterToken(u8"1234\uFFFD")},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 5uz}}}))

  TEST("PLAINTEXT", "Batches characters up to EOF then emits EOF",
       (UnitTest {.InitialState = TokenizerState::PLAINTEXT,
                  .Input = U"a string of characters; 123145",
                  .AppendEOF = true,
                  .Output = {CreateCharacterToken(u8"a string of characters; 123145"), CreateEOFToken()}}))

  TEST("PLAINTEXT", "Batches characters",
       (UnitTest {.InitialState = TokenizerState::PLAINTEXT,
                  .ExpectedState = TokenizerState::PLAINTEXT,
                  .Input = U"a string of characters; 123145",
                  .Output = {CreateCharacterToken(u8"a string of characters; 123145")}}))

#pragma endregion

#pragma region TagOpen

  TEST("TagOpen", "Switches to MarkupDeclarationOpen after parsing ExclamationMark",
       (UnitTest {.InitialState = TokenizerState::TagOpen,
                  .ExpectedState = TokenizerState::MarkupDeclarationOpen,
                  .Input = U"!",
                  .Output = {}}))

  TEST("TagOpen", "Switches to EndTagOpen after parsing Solidus",
       (UnitTest {.InitialState = TokenizerState::TagOpen,
                  .ExpectedState = TokenizerState::EndTagOpen,
                  .Input = U"/",
                  .Output = {}}))

  TEST("TagOpen", "Switches to TagName after parsing valid tag name start",
       (UnitTest {.InitialState = TokenizerState::TagOpen,
                  .ExpectedState = TokenizerState::TagName,
                  .Input = U"div",
                  .Output = {}}))

  TEST("TagOpen", "Switches to TagName after parsing valid tag name start, mixed case",
       (UnitTest {.InitialState = TokenizerState::TagOpen,
                  .ExpectedState = TokenizerState::TagName,
                  .Input = U"DiV",
                  .Output = {}}))

  TEST("TagOpen", "Switches to BogusComment after parsing QuestionMark",
       (UnitTest {
         .InitialState = TokenizerState::TagOpen,
         .ExpectedState = TokenizerState::BogusComment,
         .Input = U"?",
         .Output = {},
         .Errors = {
           {.Error = HTMLParseError::UnexpectedQuestionMarkInsteadOfTagName, .Line = 1uz, .Column = 1uz}}}))

  TEST("TagOpen", "Emits LessThanSign if EOF encountered",
       (UnitTest {.InitialState = TokenizerState::Data,
                  .Input = U"<",
                  .AppendEOF = true,
                  .Output = {CreateCharacterToken(u8"<"), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFBeforeTagName, .Line = 1uz, .Column = 2uz}}}))

  TEST("TagOpen", "Emits LessThanSign if first character invalid",
       (UnitTest {
         .InitialState = TokenizerState::TagOpen,
         .Input = U"*",
         .Output = {CreateCharacterToken(u8"<*")},
         .Errors = {{.Error = HTMLParseError::InvalidFirstCharacterOfTagName, .Line = 1uz, .Column = 1uz}}}))

#pragma endregion

#pragma region EndTagOpen

  TEST("EndTagOpen", "Switches to TagName after parsing valid tag name start",
       (UnitTest {.InitialState = TokenizerState::EndTagOpen,
                  .ExpectedState = TokenizerState::TagName,
                  .Input = U"div",
                  .Output = {}}))

  TEST("EndTagOpen", "Switches to Data if parsing GreaterThanSign (emits nothing)",
       (UnitTest {.InitialState = TokenizerState::EndTagOpen,
                  .ExpectedState = TokenizerState::Data,
                  .Input = U">",
                  .Output = {},
                  .Errors = {{.Error = HTMLParseError::MissingEndTagName, .Line = 1uz, .Column = 1uz}}}))

  TEST("EndTagOpen", "Emits LessThanSign and Solidus if EOF encountered",
       (UnitTest {.InitialState = TokenizerState::TagOpen,
                  .Input = U"/",
                  .AppendEOF = true,
                  .Output = {CreateCharacterToken(u8"</"), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFBeforeTagName, .Line = 1uz, .Column = 2uz}}}))

  TEST("EndTagOpen", "Switches to BogusComment if first character invalid",
       (UnitTest {
         .InitialState = TokenizerState::EndTagOpen,
         .ExpectedState = TokenizerState::BogusComment,
         .Input = U"*",
         .Output = {},
         .Errors = {{.Error = HTMLParseError::InvalidFirstCharacterOfTagName, .Line = 1uz, .Column = 1uz}}}))

#pragma endregion

#pragma region TagName

  TEST("TagName", "Emits tag token when parsing GreaterThanSign",
       (UnitTest {.Input = U"<div>", .Output = {CreateStartTagToken({.Name = u8"div"})}}))

  TEST("TagName", "Allows mixed case tag name",
       (UnitTest {.Input = U"<DiV>", .Output = {CreateStartTagToken({.Name = u8"div"})}}))

  TEST("TagName", "Switches to BeforeAttributeName when parsing whitespace",
       (UnitTest {.ExpectedState = TokenizerState::BeforeAttributeName, .Input = U"<div "}))

  TEST("TagName", "Switches to BeforeAttributeName when parsing solidus",
       (UnitTest {.ExpectedState = TokenizerState::SelfClosingStartTag, .Input = U"<div/"}))

  TEST("TagName", "Emits null character as U+FFFD with parse error",
       (UnitTest {
         .Input = InsertUTF32Null(U"<div", U">"),
         .Output = {CreateStartTagToken({.Name = u8"div\uFFFD"})},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 5uz}}}))

  TEST("TagName", "Emits EOF instead of tag token if EOF reached",
       (UnitTest {.Input = U"<div",
                  .AppendEOF = true,
                  .Output = {CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInTag, .Line = 1uz, .Column = 5uz}}}))

#pragma endregion

#pragma region RCDATALessThanSign

  TEST("RCDATALessThanSign", "Switches to RCDATAEndTagOpen after parsing solidus",
       (UnitTest {.InitialState = TokenizerState::RCDATALessThanSign,
                  .ExpectedState = TokenizerState::RCDATAEndTagOpen,
                  .Input = U"/"}))

  TEST("RCDATALessThanSign", "emits less than sign after parsing non solidus",
       (UnitTest {.InitialState = TokenizerState::RCDATALessThanSign,
                  .ExpectedState = TokenizerState::RCDATA,
                  .Input = U"©",
                  .Output = {CreateCharacterToken(u8"<©")}}))

  TEST("RCDATALessThanSign", "emits less than sign if EOF reached",
       (UnitTest {.InitialState = TokenizerState::RCDATA,
                  .Input = U"<",
                  .AppendEOF = true,
                  .Output = {CreateCharacterToken(u8"<"), CreateEOFToken()}}))

#pragma endregion

#pragma region RCDATAEndTagOpen

  TEST("RCDATAEndTagOpen", "Switches to RCDATAEndTagName after parsing alpha",
       (UnitTest {.InitialState = TokenizerState::RCDATAEndTagOpen,
                  .ExpectedState = TokenizerState::RCDATAEndTagName,
                  .Input = U"a"}))

  TEST("RCDATAEndTagOpen", "emits less than sign and solidus after parsing non alpha",
       (UnitTest {.InitialState = TokenizerState::RCDATAEndTagOpen,
                  .ExpectedState = TokenizerState::RCDATA,
                  .Input = U"©",
                  .Output = {CreateCharacterToken(u8"</©")}}))

  TEST("RCDATAEndTagOpen", "emits less than sign and solidus if EOF reached",
       (UnitTest {.InitialState = TokenizerState::RCDATA,
                  .Input = U"</",
                  .AppendEOF = true,
                  .Output = {CreateCharacterToken(u8"</"), CreateEOFToken()}}))

#pragma endregion

#pragma region RCDATAEndTagName

  TEST("RCDATAEndTagName", "Emits less than sign, solidus, and characters when tag name doesn't match",
       (UnitTest {.InitialState = TokenizerState::RCDATAEndTagName,
                  .ExpectedState = TokenizerState::RCDATA,
                  .Input = U"span©",
                  .Output = {CreateCharacterToken(u8"</span©")}}))

  TEST("RCDATAEndTagName", "Emits less than sign, solidus, and characters when EOF reached",
       (UnitTest {.InitialState = TokenizerState::RCDATAEndTagName,
                  .Input = U"di",
                  .AppendEOF = true,
                  .Output = {CreateCharacterToken(u8"</di"), CreateEOFToken()}}))

  TEST("RCDATAEndTagName",
       "Emits less than sign, solidus, and characters when invalid tag name character encountered",
       (UnitTest {.InitialState = TokenizerState::RCDATAEndTagName,
                  .ExpectedState = TokenizerState::RCDATA,
                  .Input = U"d!v>",
                  .Output = {CreateCharacterToken(u8"</d!v>")}}))

  TEST_CASE("HTMLTokenizer(RCDATAEndTagName) - switches to BeforeAttributeName when parsing whitespace and "
            "end tag name matches start tag",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    inputStream.Append(U"<div></div ");

    // Start tag
    expected = u8"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::RCDATA);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(tokenizer.State() == TokenizerState::BeforeAttributeName);
    }
  }

  TEST_CASE("HTMLTokenizer(RCDATAEndTagName) - switches to SelfClosingStartTag when parsing solidus and "
            "end tag name matches start tag",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<div></div/");

    // Start tag
    expected = u8"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::RCDATA);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(tokenizer.State() == TokenizerState::SelfClosingStartTag);
    }
  }

  TEST_CASE("HTMLTokenizer(RCDATAEndTagName) - emits end tag token when parsing GreaterThanSign end tag name "
            "matches start tag",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    inputStream.Append(U"<div></div>");

    // Start tag
    expected = u8"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::RCDATA);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::EndTag);
    }
  }

#pragma endregion

#pragma region RAWTEXTLessThanSign

  TEST("RAWTEXTLessThanSign", "Switches to RAWTEXTEndTagOpen after parsing solidus",
       (UnitTest {.InitialState = TokenizerState::RAWTEXTLessThanSign,
                  .ExpectedState = TokenizerState::RAWTEXTEndTagOpen,
                  .Input = U"/"}))

  TEST("RAWTEXTLessThanSign", "emits less than sign after parsing non solidus",
       (UnitTest {.InitialState = TokenizerState::RAWTEXTLessThanSign,
                  .ExpectedState = TokenizerState::RAWTEXT,
                  .Input = U"©",
                  .Output = {{CreateCharacterToken(u8"<©")}}}))

  TEST("RAWTEXTLessThanSign", "Emits LessThanSign if EOF reached",
       (UnitTest {.InitialState = TokenizerState::RAWTEXT,
                  .Input = U"<",
                  .AppendEOF = true,
                  .Output = {{CreateCharacterToken(u8"<"), CreateEOFToken()}}}))

#pragma endregion

#pragma region RAWTEXTEndTagOpen

  TEST("RAWTEXTEndTagOpen", "Switches to RAWTEXTEndTagName after parsing valid tag name start",
       (UnitTest {.InitialState = TokenizerState::RAWTEXTEndTagOpen,
                  .ExpectedState = TokenizerState::RAWTEXTEndTagName,
                  .Input = U"a"}))

  TEST("RAWTEXTEndTagOpen", "emits less than sign and solidus after parsing invalid tag name start",
       (UnitTest {.InitialState = TokenizerState::RAWTEXTEndTagOpen,
                  .ExpectedState = TokenizerState::RAWTEXT,
                  .Input = U"©",
                  .Output = {CreateCharacterToken(u8"</©")}}))

  TEST("RAWTEXTEndTagOpen", "emits less than sign and solidus if EOF reached",
       (UnitTest {.InitialState = TokenizerState::RAWTEXT,
                  .Input = U"</",
                  .AppendEOF = true,
                  .Output = {CreateCharacterToken(u8"</")}}))

#pragma endregion

#pragma region RAWTEXTEndTagName

  TEST_CASE("HTMLTokenizer(RAWTEXTEndTagName) - switches to BeforeAttributeName when parsing whitespace and "
            "end tag name matches start tag",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<div></div ");

    // Start tag
    expected = u8"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::RAWTEXT);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(tokenizer.State() == TokenizerState::BeforeAttributeName);
    }
  }

  TEST_CASE("HTMLTokenizer(RAWTEXTEndTagName) - switches to SelfClosingStartTag when parsing solidus and "
            "end tag name matches start tag",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<div></div/");

    // Start tag
    expected = u8"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::RAWTEXT);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(tokenizer.State() == TokenizerState::SelfClosingStartTag);
    }
  }

  TEST_CASE(
    "HTMLTokenizer(RAWTEXTEndTagName) - emits end tag token when parsing GreaterThanSign end tag name "
    "matches start tag",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<div></div>");

    // Start tag
    expected = u8"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::RAWTEXT);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::EndTag);
    }
  }

  TEST_CASE("HTMLTokenizer(RAWTEXTEndTagName) - allows mixed case", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<div></DiV>");

    // Start tag
    expected = u8"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::RAWTEXT);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::EndTag);
    }
  }

  TEST_CASE("HTMLTokenizer(RAWTEXTEndTagName) - emits less than sign, solidus, and characters when tag name "
            "doesn't match",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<div></span©", IsEOF(true));

    // Start tag
    expected = u8"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::RAWTEXT);

    expected = u8"</span©";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::Character);
    }
  }

#pragma endregion

#pragma region ScriptDataLessThanSign

  TEST("ScriptDataLessThanSign", "Switches to ScriptDataEndTagOpen after parsing solidus",
       (UnitTest {.InitialState = TokenizerState::ScriptDataLessThanSign,
                  .ExpectedState = TokenizerState::ScriptDataEndTagOpen,
                  .Input = U"/"}))

  TEST("ScriptDataLessThanSign", "emits LessThanSign after parsing non solidus",
       (UnitTest {.InitialState = TokenizerState::ScriptDataLessThanSign,
                  .ExpectedState = TokenizerState::ScriptData,
                  .Input = U"©",
                  .Output = {CreateCharacterToken(u8"<©")}}))

  TEST("ScriptDataLessThanSign", "Emits LessThanSign if EOF reached",
       (UnitTest {.InitialState = TokenizerState::ScriptData,
                  .Input = U"<",
                  .AppendEOF = true,
                  .Output = {CreateCharacterToken(u8"<"), CreateEOFToken()}}))

  TEST("ScriptDataLessThanSign",
       "Switches to ScriptDataEscapeStart after parsing ExclamationMark and emits LessThanSign and "
       "ExclamationMark",
       (UnitTest {.InitialState = TokenizerState::ScriptDataLessThanSign,
                  .ExpectedState = TokenizerState::ScriptDataEscapeStart,
                  .Input = U"!",
                  .Output = {CreateCharacterToken(u8"<!")}}))

#pragma endregion

#pragma region ScriptDataEndTagOpen

  TEST("ScriptDataEndTagOpen", "Switches to ScriptDataEndTagName after parsing alpha",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEndTagOpen,
                  .ExpectedState = TokenizerState::ScriptDataEndTagName,
                  .Input = U"a"}))

  TEST("ScriptDataEndTagOpen", "emits less than sign and solidus after parsing non alpha",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEndTagOpen,
                  .ExpectedState = TokenizerState::ScriptData,
                  .Input = U"©",
                  .Output = {CreateCharacterToken(u8"</©")}}))

  TEST("ScriptDataEndTagOpen", "emits less than sign and solidus if EOF reached",
       (UnitTest {.InitialState = TokenizerState::ScriptData,
                  .Input = U"</",
                  .AppendEOF = true,
                  .Output = {CreateCharacterToken(u8"</")}}))

#pragma endregion

#pragma region ScriptDataEndTagName

  TEST_CASE(
    "HTMLTokenizer(ScriptDataEndTagName) - switches to BeforeAttributeName when parsing whitespace and "
    "end tag name matches start tag",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></script ");

    // Start tag
    expected = u8"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::ScriptData);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(tokenizer.State() == TokenizerState::BeforeAttributeName);
    }
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEndTagName) - switches to SelfClosingStartTag when parsing solidus and "
            "end tag name matches start tag",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></script/");

    // Start tag
    expected = u8"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::ScriptData);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(tokenizer.State() == TokenizerState::SelfClosingStartTag);
    }
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataEndTagName) - emits end tag token when parsing GreaterThanSign end tag name "
    "matches start tag",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></script>");

    // Start tag
    expected = u8"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::ScriptData);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::EndTag);
    }
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEndTagName) - allows mixed case", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></ScRiPt>");

    // Start tag
    expected = u8"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::ScriptData);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::EndTag);
    }
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataEndTagName) - emits less than sign, solidus, and characters when tag name "
    "doesn't match",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></span©", IsEOF(true));

    // Start tag
    expected = u8"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::ScriptData);

    expected = u8"</span©";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::Character);
    }
  }

#pragma endregion

#pragma region ScriptDataEscapeStart

  TEST("ScriptDataEscapeStart", "switches to ScriptDataEscapeStartDash after parsing HyphenMinus",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscapeStart,
                  .ExpectedState = TokenizerState::ScriptDataEscapeStartDash,
                  .Input = U"-",
                  .Output = {CreateCharacterToken(u8"-")}}))

  TEST("ScriptDataEscapeStart", "Replaces null character with U+FFFD and switches to ScriptData",
       (UnitTest {
         .InitialState = TokenizerState::ScriptDataEscapeStart,
         .ExpectedState = TokenizerState::ScriptData,
         .Input = InsertUTF32Null(U"1234"),
         .Output = {CreateCharacterToken(u8"1234\uFFFD")},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 5uz}}}))

  TEST("ScriptDataEscapeStart", "Batches characters up to EOF then emits EOF",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscapeStart,
                  .Input = U"a string of characters; 123145",
                  .AppendEOF = true,
                  .Output = {CreateCharacterToken(u8"a string of characters; 123145"), CreateEOFToken()}}))

  TEST("ScriptDataEscapeStart", "switches to ScriptData after parsing non HyphenMinus",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscapeStart,
                  .ExpectedState = TokenizerState::ScriptData,
                  .Input = U"©",
                  .Output = {CreateCharacterToken(u8"©")}}))

#pragma endregion

#pragma region ScriptDataEscaped

  TEST("ScriptDataEscaped", "Batches characters",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscaped,
                  .ExpectedState = TokenizerState::ScriptDataEscaped,
                  .Input = U"a string of characters; 123145",
                  .Output = {CreateCharacterToken(u8"a string of characters; 123145")}}))

  TEST("ScriptDataEscaped", "Batches characters up to EOF then emits EOF",
       (UnitTest {
         .InitialState = TokenizerState::ScriptDataEscaped,
         .Input = U"a string of characters; 123145",
         .AppendEOF = true,
         .Output = {CreateCharacterToken(u8"a string of characters; 123145"), CreateEOFToken()},
         .Errors = {{.Error = HTMLParseError::EOFInScriptHTMLCommentLikeText, .Line = 1uz, .Column = 31uz}}}))

  TEST("ScriptDataEscaped", "Replaces null character with U+FFFD",
       (UnitTest {
         .InitialState = TokenizerState::ScriptDataEscaped,
         .ExpectedState = TokenizerState::ScriptDataEscaped,
         .Input = InsertUTF32Null(U"1234"),
         .Output = {CreateCharacterToken(u8"1234\uFFFD")},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 5uz}}}))

  TEST("ScriptDataEscaped", "switches to ScriptDataEscapedLessThanSign when parsing LessThanSign",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscaped,
                  .ExpectedState = TokenizerState::ScriptDataEscapedLessThanSign,
                  .Input = U"<"}))

  TEST("ScriptDataEscaped", "switches to ScriptDataEscapedDash when parsing HyphenMinus",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscaped,
                  .ExpectedState = TokenizerState::ScriptDataEscapedDash,
                  .Input = U"-",
                  .Output = {CreateCharacterToken(u8"-")}}))

#pragma endregion

#pragma region ScriptDataEscapedDash

  TEST("ScriptDataEscapedDash", "Batches characters",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscapedDash,
                  .ExpectedState = TokenizerState::ScriptDataEscaped,
                  .Input = U"a string of characters; 123145",
                  .Output = {CreateCharacterToken(u8"a string of characters; 123145")}}))

  TEST("ScriptDataEscapedDash", "Batches characters up to EOF then emits EOF",
       (UnitTest {
         .InitialState = TokenizerState::ScriptDataEscapedDash,
         .Input = U"a string of characters; 123145",
         .AppendEOF = true,
         .Output = {CreateCharacterToken(u8"a string of characters; 123145"), CreateEOFToken()},
         .Errors = {{.Error = HTMLParseError::EOFInScriptHTMLCommentLikeText, .Line = 1uz, .Column = 31uz}}}))

  TEST("ScriptDataEscapedDash", "Replaces null character with U+FFFD",
       (UnitTest {
         .InitialState = TokenizerState::ScriptDataEscapedDash,
         .ExpectedState = TokenizerState::ScriptDataEscaped,
         .Input = InsertUTF32Null(U"1234"),
         .Output = {CreateCharacterToken(u8"1234\uFFFD")},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 5uz}}}))

  TEST("ScriptDataEscapedDash", "switches to ScriptDataEscapedLessThanSign when parsing LessThanSign",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscapedDash,
                  .ExpectedState = TokenizerState::ScriptDataEscapedLessThanSign,
                  .Input = U"<"}))

  TEST("ScriptDataEscapedDash", "switches to ScriptDataEscapedDashDash when parsing HyphenMinus",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscapedDash,
                  .ExpectedState = TokenizerState::ScriptDataEscapedDashDash,
                  .Input = U"-",
                  .Output = {CreateCharacterToken(u8"-")}}))

  TEST("ScriptDataEscapedDash",
       "switches to ScriptDataEscaped when parsing non HyphenMinus or "
       "LessThanSign",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscapedDash,
                  .ExpectedState = TokenizerState::ScriptDataEscaped,
                  .Input = U"©",
                  .Output = {CreateCharacterToken(u8"©")}}))

#pragma endregion

#pragma region ScriptDataEscapedDashDash

  TEST("ScriptDataEscapedDashDash", "Batches characters",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscapedDashDash,
                  .ExpectedState = TokenizerState::ScriptDataEscaped,
                  .Input = U"a string of characters; 123145",
                  .Output = {CreateCharacterToken(u8"a string of characters; 123145")}}))

  TEST("ScriptDataEscapedDashDash", "Batches characters up to EOF then emits EOF",
       (UnitTest {
         .InitialState = TokenizerState::ScriptDataEscapedDashDash,
         .Input = U"a string of characters; 123145",
         .AppendEOF = true,
         .Output = {CreateCharacterToken(u8"a string of characters; 123145"), CreateEOFToken()},
         .Errors = {{.Error = HTMLParseError::EOFInScriptHTMLCommentLikeText, .Line = 1uz, .Column = 31uz}}}))

  TEST("ScriptDataEscapedDashDash", "Replaces null character with U+FFFD and switches to ScriptDataEscaped",
       (UnitTest {
         .InitialState = TokenizerState::ScriptDataEscapedDashDash,
         .ExpectedState = TokenizerState::ScriptDataEscaped,
         .Input = InsertUTF32Null(U"1234"),
         .Output = {CreateCharacterToken(u8"1234\uFFFD")},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 5uz}}}))

  TEST("ScriptDataEscapedDashDash", "switches to ScriptDataEscapedLessThanSign when parsing LessThanSign",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscapedDashDash,
                  .ExpectedState = TokenizerState::ScriptDataEscapedLessThanSign,
                  .Input = U"<"}))

  TEST("ScriptDataEscapedDashDash", "switches to ScriptData when parsing GreaterThanSign",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscapedDashDash,
                  .ExpectedState = TokenizerState::ScriptData,
                  .Input = U">",
                  .Output = {CreateCharacterToken(u8">")}}))

  TEST("ScriptDataEscapedDashDash", "emits HyphenMinus and stays in the same state when parsing HyphenMinus",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscapedDashDash,
                  .ExpectedState = TokenizerState::ScriptDataEscapedDashDash,
                  .Input = U"-",
                  .Output = {CreateCharacterToken(u8"-")}}))

#pragma endregion

#pragma region ScriptDataEscapedLessThanSign

  TEST("ScriptDataEscapedLessThanSign",
       "Replaces null character with U+FFFD and switches to ScriptDataEscaped",
       (UnitTest {
         .InitialState = TokenizerState::ScriptDataEscapedLessThanSign,
         .ExpectedState = TokenizerState::ScriptDataEscaped,
         .Input = InsertUTF32Null(U"1234"),
         .Output = {CreateCharacterToken(u8"<1234\uFFFD")},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 5uz}}}))

  TEST("ScriptDataEscapedLessThanSign", "Batches characters up to EOF then emits EOF",
       (UnitTest {
         .InitialState = TokenizerState::ScriptDataEscapedLessThanSign,
         .Input = U"a string of characters; 123145",
         .AppendEOF = true,
         .Output = {CreateCharacterToken(u8"<a string of characters; 123145"), CreateEOFToken()},
         .Errors = {{.Error = HTMLParseError::EOFInScriptHTMLCommentLikeText, .Line = 1uz, .Column = 31uz}}}))

  TEST("ScriptDataEscapedLessThanSign",
       "emits LessThanSign and switches to ScriptDataEscapedEndTagOpen after parsing Solidus",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscapedLessThanSign,
                  .ExpectedState = TokenizerState::ScriptDataEscapedEndTagOpen,
                  .Input = U"/",
                  .Output = {}}))

  TEST("ScriptDataEscapedLessThanSign",
       "emits LessThanSign and next character and switches to ScriptDataDoubleEscapeStart if ASCII alpha ",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscapedLessThanSign,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscapeStart,
                  .Input = U"A",
                  .Output = {CreateCharacterToken(u8"<A")}}))

  TEST("ScriptDataEscapedLessThanSign",
       "emits LessThanSign and next character and switches to ScriptDataEscaped if not ASCII alpha ",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscapedLessThanSign,
                  .ExpectedState = TokenizerState::ScriptDataEscaped,
                  .Input = U"©",
                  .Output = {CreateCharacterToken(u8"<©")}}))

#pragma endregion

#pragma region ScriptDataEscapedEndTagOpen

  TEST("ScriptDataEscapedEndTagOpen", "Switches to ScriptDataEscapedEndTagName after parsing alpha",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscapedEndTagOpen,
                  .ExpectedState = TokenizerState::ScriptDataEscapedEndTagName,
                  .Input = U"a"}))

  TEST("ScriptDataEscapedEndTagOpen", "emits less than sign and solidus after parsing non alpha",
       (UnitTest {.InitialState = TokenizerState::ScriptDataEscapedEndTagOpen,
                  .ExpectedState = TokenizerState::ScriptDataEscaped,
                  .Input = U"©",
                  .Output = {CreateCharacterToken(u8"</©")}}))

  TEST("ScriptDataEscapedEndTagOpen", "emits less than sign and solidus if EOF reached",
       (UnitTest {
         .InitialState = TokenizerState::ScriptDataEscaped,
         .Input = U"</",
         .AppendEOF = true,
         .Output = {CreateCharacterToken(u8"</"), CreateEOFToken()},
         .Errors = {{.Error = HTMLParseError::EOFInScriptHTMLCommentLikeText, .Line = 1uz, .Column = 3uz}}}))

#pragma endregion

#pragma region ScriptDataEscapedEndTagName

  TEST_CASE(
    "HTMLTokenizer(ScriptDataEscapedEndTagName) - switches to BeforeAttributeName when parsing whitespace "
    "and end tag name matches start tag",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></script ");

    // Start tag
    expected = u8"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::ScriptDataEscaped);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(tokenizer.State() == TokenizerState::BeforeAttributeName);
    }
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedEndTagName) - switches to SelfClosingStartTag when parsing "
            "solidus and end tag name matches start tag",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></script/");

    // Start tag
    expected = u8"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::ScriptDataEscaped);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(tokenizer.State() == TokenizerState::SelfClosingStartTag);
    }
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataEscapedEndTagName) - emits end tag token when parsing GreaterThanSign end "
    "tag name matches start tag",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></script>");

    // Start tag
    expected = u8"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::ScriptDataEscaped);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::EndTag);
    }
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedEndTagName) - allows mixed case", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></ScRiPt>");

    // Start tag
    expected = u8"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::ScriptDataEscaped);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::EndTag);
    }
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedEndTagName) - emits less than sign, solidus, and characters when "
            "tag name doesn't match",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></span©");

    // Start tag
    expected = u8"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::StartTag);
    }

    tokenizer.State(TokenizerState::ScriptDataEscaped);

    expected = u8"</span©";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLTokenType::Character);
    }
  }

#pragma endregion

#pragma region ScriptDataDoubleEscapeStart

  TEST("ScriptDataDoubleEscapeStart",
       "switches to ScriptDataEscaped after parsing whitespace if temporary buffer is not 'script'",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapeStart,
                  .ExpectedState = TokenizerState::ScriptDataEscaped,
                  .Input = U" ",
                  .Output = {CreateCharacterToken(u8" ")}}))

  TEST("ScriptDataDoubleEscapeStart",
       "switches to ScriptDataEscaped after parsing Solidus if temporary buffer is not 'script'",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapeStart,
                  .ExpectedState = TokenizerState::ScriptDataEscaped,
                  .Input = U"/",
                  .Output = {CreateCharacterToken(u8"/")}}))

  TEST("ScriptDataDoubleEscapeStart",
       "switches to ScriptDataEscaped after parsing GreaterThanSign if temporary buffer is not 'script'",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapeStart,
                  .ExpectedState = TokenizerState::ScriptDataEscaped,
                  .Input = U">",
                  .Output = {CreateCharacterToken(u8">")}}))

  TEST("ScriptDataDoubleEscapeStart", "continues building temporary buffer when parsing ASCII alpha",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapeStart,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscapeStart,
                  .Input = U"sCr",
                  .Output = {CreateCharacterToken(u8"sCr")}}))

  TEST("ScriptDataDoubleEscapeStart", "switches to ScriptDataEscaped if not ASCII alpha ",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapeStart,
                  .ExpectedState = TokenizerState::ScriptDataEscaped,
                  .Input = U"©",
                  .Output = {CreateCharacterToken(u8"©")}}))

  TEST("ScriptDataDoubleEscapeStart",
       "switches to ScriptDataDoubleEscaped after parsing whitespace if temporary buffer is 'script'",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapeStart,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscaped,
                  .Input = U"script ",
                  .Output = {CreateCharacterToken(u8"script ")}}))

  TEST("ScriptDataDoubleEscapeStart",
       "switches to ScriptDataDoubleEscaped after parsing Solidus if temporary buffer is 'script'",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapeStart,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscaped,
                  .Input = U"script/",
                  .Output = {CreateCharacterToken(u8"script/")}}))

  TEST("ScriptDataDoubleEscapeStart",
       "switches to ScriptDataDoubleEscaped after parsing GreaterThanSign if temporary buffer is 'script'",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapeStart,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscaped,
                  .Input = U"script>",
                  .Output = {CreateCharacterToken(u8"script>")}}))

#pragma endregion

#pragma region ScriptDataDoubleEscaped

  TEST("ScriptDataDoubleEscaped", "Batches characters",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscaped,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscaped,
                  .Input = U"a string of characters; 123145",
                  .Output = {CreateCharacterToken(u8"a string of characters; 123145")}}))

  TEST("ScriptDataDoubleEscaped", "Batches characters up to EOF then emits EOF",
       (UnitTest {
         .InitialState = TokenizerState::ScriptDataDoubleEscaped,
         .Input = U"a string of characters; 123145",
         .AppendEOF = true,
         .Output = {CreateCharacterToken(u8"a string of characters; 123145"), CreateEOFToken()},
         .Errors = {{.Error = HTMLParseError::EOFInScriptHTMLCommentLikeText, .Line = 1uz, .Column = 31uz}}}))

  TEST("ScriptDataDoubleEscaped", "Replaces null character with U+FFFD",
       (UnitTest {
         .InitialState = TokenizerState::ScriptDataDoubleEscaped,
         .ExpectedState = TokenizerState::ScriptDataDoubleEscaped,
         .Input = InsertUTF32Null(U"1234"),
         .Output = {CreateCharacterToken(u8"1234\uFFFD")},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 5uz}}}))

  TEST("ScriptDataDoubleEscaped", "switches to ScriptDataDoubleEscapedLessThanSign when parsing LessThanSign",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscaped,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscapedLessThanSign,
                  .Input = U"<",
                  .Output = {CreateCharacterToken(u8"<")}}))

  TEST("ScriptDataDoubleEscaped", "switches to ScriptDataDoubleEscapedDash when parsing HyphenMinus",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscaped,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscapedDash,
                  .Input = U"-",
                  .Output = {CreateCharacterToken(u8"-")}}))

#pragma endregion

#pragma region ScriptDataDoubleEscapedDash

  TEST("ScriptDataDoubleEscapedDash",
       "Switches to ScriptDataDoubleEscaped after parsing any other character and emits that character",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapedDash,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscaped,
                  .Input = U"©",
                  .Output = {CreateCharacterToken(u8"©")}}))

  TEST("ScriptDataDoubleEscapedDash", "Batches characters up to EOF then emits EOF",
       (UnitTest {
         .InitialState = TokenizerState::ScriptDataDoubleEscapedDash,
         .Input = U"a string of characters; 123145",
         .AppendEOF = true,
         .Output = {CreateCharacterToken(u8"a string of characters; 123145"), CreateEOFToken()},
         .Errors = {{.Error = HTMLParseError::EOFInScriptHTMLCommentLikeText, .Line = 1uz, .Column = 31uz}}}))

  TEST("ScriptDataDoubleEscapedDash", "Replaces null character with U+FFFD",
       (UnitTest {
         .InitialState = TokenizerState::ScriptDataDoubleEscapedDash,
         .ExpectedState = TokenizerState::ScriptDataDoubleEscaped,
         .Input = InsertUTF32Null(U"1234"),
         .Output = {CreateCharacterToken(u8"1234\uFFFD")},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 5uz}}}))

  TEST("ScriptDataDoubleEscapedDash",
       "switches to ScriptDataDoubleEscapedLessThanSign when parsing LessThanSign",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapedDash,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscapedLessThanSign,
                  .Input = U"<",
                  .Output = {CreateCharacterToken(u8"<")}}))

  TEST("ScriptDataDoubleEscapedDash", "switches to ScriptDataDoubleEscapedDashDash when parsing HyphenMinus",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapedDash,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscapedDashDash,
                  .Input = U"-",
                  .Output = {CreateCharacterToken(u8"-")}}))

#pragma endregion

#pragma region ScriptDataDoubleEscapedDashDash

  TEST("ScriptDataDoubleEscapedDashDash",
       "Switches to ScriptDataDoubleEscaped after parsing any other character and emits that character",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapedDashDash,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscaped,
                  .Input = U"©",
                  .Output = {CreateCharacterToken(u8"©")}}))

  TEST("ScriptDataDoubleEscapedDashDash", "emits EOF with parser error when EOF reached",
       (UnitTest {
         .InitialState = TokenizerState::ScriptDataDoubleEscapedDashDash,
         .Input = U"-",
         .AppendEOF = true,
         .Output = {CreateCharacterToken(u8"-"), CreateEOFToken()},
         .Errors = {{.Error = HTMLParseError::EOFInScriptHTMLCommentLikeText, .Line = 1uz, .Column = 2uz}}}))

  TEST("ScriptDataDoubleEscapedDashDash",
       "Replaces null character with U+FFFD and switches to ScriptDataDoubleEscaped",
       (UnitTest {
         .InitialState = TokenizerState::ScriptDataDoubleEscapedDashDash,
         .ExpectedState = TokenizerState::ScriptDataDoubleEscaped,
         .Input = InsertUTF32Null(U"1234"),
         .Output = {CreateCharacterToken(u8"1234\uFFFD")},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 5uz}}}))

  TEST("ScriptDataDoubleEscapedDashDash",
       "switches to ScriptDataDoubleEscapedLessThanSign when parsing LessThanSign",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapedDashDash,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscapedLessThanSign,
                  .Input = U"<",
                  .Output = {CreateCharacterToken(u8"<")}}))

  TEST("ScriptDataDoubleEscapedDashDash", "switches to ScriptData after parsing GreaterThanSign",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapedDashDash,
                  .ExpectedState = TokenizerState::ScriptData,
                  .Input = U">",
                  .Output = {CreateCharacterToken(u8">")}}))

  TEST("ScriptDataDoubleEscapedDashDash",
       "emits HyphenMinus and stays in the same state when parsing HyphenMinus",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapedDashDash,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscapedDashDash,
                  .Input = U"-",
                  .Output = {CreateCharacterToken(u8"-")}}))

#pragma endregion

#pragma region ScriptDataDoubleEscapedLessThanSign

  TEST("ScriptDataDoubleEscapedLessThanSign",
       "Replaces null character with U+FFFD and switches to ScriptDataDoubleEscaped",
       (UnitTest {
         .InitialState = TokenizerState::ScriptDataDoubleEscapedLessThanSign,
         .ExpectedState = TokenizerState::ScriptDataDoubleEscaped,
         .Input = InsertUTF32Null(U"1234"),
         .Output = {CreateCharacterToken(u8"1234\uFFFD")},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 5uz}}}))

  TEST("ScriptDataDoubleEscapedLessThanSign",
       "Switches to ScriptDataDoubleEscapeEnd after parsing Solidus and emits Solidus",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapedLessThanSign,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscapeEnd,
                  .Input = U"/",
                  .Output = {CreateCharacterToken(u8"/")}}))

  TEST("ScriptDataDoubleEscapedLessThanSign",
       "Switches to ScriptDataDoubleEscaped after parsing any other character and emits that character",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapedLessThanSign,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscaped,
                  .Input = U"A",
                  .Output = {CreateCharacterToken(u8"A")}}))

#pragma endregion

#pragma region ScriptDataDoubleEscapeEnd

  TEST("ScriptDataDoubleEscapeEnd",
       "switches to ScriptDataDoubleEscaped after parsing Solidus if temporary buffer is not 'script'",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapeEnd,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscaped,
                  .Input = U"/",
                  .Output = {CreateCharacterToken(u8"/")}}))

  TEST(
    "ScriptDataDoubleEscapeEnd",
    "switches to ScriptDataDoubleEscaped after parsing GreaterThanSign if temporary buffer is not 'script'",
    (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapeEnd,
               .ExpectedState = TokenizerState::ScriptDataDoubleEscaped,
               .Input = U">",
               .Output = {CreateCharacterToken(u8">")}}))

  TEST("ScriptDataDoubleEscapeEnd",
       "switches to ScriptDataDoubleEscaped after parsing whitespace if temporary buffer is not 'script'",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapeEnd,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscaped,
                  .Input = U" ",
                  .Output = {CreateCharacterToken(u8" ")}}))

  TEST("ScriptDataDoubleEscapeEnd",
       "switches to ScriptDataEscaped after parsing whitespace if temporary buffer is 'script'",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapeEnd,
                  .ExpectedState = TokenizerState::ScriptDataEscaped,
                  .Input = U"script ",
                  .Output = {CreateCharacterToken(u8"script ")}}))

  TEST("ScriptDataDoubleEscapeEnd",
       "switches to ScriptDataEscaped after parsing Solidus if temporary buffer is 'script'",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapeEnd,
                  .ExpectedState = TokenizerState::ScriptDataEscaped,
                  .Input = U"script/",
                  .Output = {CreateCharacterToken(u8"script/")}}))

  TEST("ScriptDataDoubleEscapeEnd",
       "switches to ScriptDataEscaped after parsing GreaterThanSign if temporary buffer is 'script'",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapeEnd,
                  .ExpectedState = TokenizerState::ScriptDataEscaped,
                  .Input = U"script>",
                  .Output = {CreateCharacterToken(u8"script>")}}))

  TEST("ScriptDataDoubleEscapeEnd", "continues building temporary buffer when parsing ASCII alpha",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapeEnd,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscapeEnd,
                  .Input = U"sCr",
                  .Output = {CreateCharacterToken(u8"sCr")}}))

  TEST("ScriptDataDoubleEscapeEnd", "switches to ScriptDataDoubleEscaped if not ASCII alpha ",
       (UnitTest {.InitialState = TokenizerState::ScriptDataDoubleEscapeEnd,
                  .ExpectedState = TokenizerState::ScriptDataDoubleEscaped,
                  .Input = U"©",
                  .Output = {CreateCharacterToken(u8"©")}}))

#pragma endregion

#pragma region BeforeAttributeName

  TEST("BeforeAttributeName", "ignores whitespace and stays in the same state when parsing whitespace",
       (UnitTest {.InitialState = TokenizerState::BeforeAttributeName,
                  .ExpectedState = TokenizerState::BeforeAttributeName,
                  .Input = U"   \t\n\r"}))

  TEST("BeforeAttributeName", "switches to SelfClosingStartTag when parsing Solidus",
       (UnitTest {.InitialState = TokenizerState::BeforeAttributeName,
                  .ExpectedState = TokenizerState::SelfClosingStartTag,
                  .Input = U"/"}))

  TEST("BeforeAttributeName", "switches to Data and emits the current tag when parsing GreaterThanSign",
       (UnitTest {.InitialState = TokenizerState::Data,
                  .Input = U"<div >",
                  .Output = {CreateStartTagToken({.Name = u8"div"})}}))

  TEST("BeforeAttributeName", "switches to Data and emits EOF when EOF reached",
       (UnitTest {.InitialState = TokenizerState::Data,
                  .Input = U"<div ",
                  .AppendEOF = true,
                  .Output = {CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInTag, .Line = 1uz, .Column = 6uz}}}))

  TEST("BeforeAttributeName", "switches to AttributeName with parser error when parsing an EqualsSign",
       (UnitTest {
         .InitialState = TokenizerState::Data,
         .ExpectedState = TokenizerState::AttributeName,
         .Input = U"<a =",
         .Errors = {
           {.Error = HTMLParseError::UnexpectedEqualsSignBeforeAttributeName, .Line = 1uz, .Column = 4uz}}}))

  TEST("BeforeAttributeName", "switches to AttributeName after parsing valid attribute name start",
       (UnitTest {.InitialState = TokenizerState::Data,
                  .ExpectedState = TokenizerState::AttributeName,
                  .Input = U"<a b"}))

#pragma endregion

#pragma region AttributeName

  TEST("AttributeName", "switches to Data and emits the current tag when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<div b>",
         .Output = {CreateStartTagToken({.Name = u8"div", .Attributes = {{.Name = u8"b", .Value = u8""}}})}}))

  TEST("AttributeName", "switches to Data and emits EOF when EOF reached",
       (UnitTest {.Input = U"<div b",
                  .AppendEOF = true,
                  .Output = {CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInTag, .Line = 1uz, .Column = 7uz}}}))

  TEST("AttributeName",
       "switches to Data and emits the current tag with parser error when parsing Solidus GreaterThanSign",
       (UnitTest {.Input = U"<div b/>",
                  .Output = {CreateStartTagToken({.Name = u8"div",
                                                  .Attributes = {{.Name = u8"b", .Value = u8""}},
                                                  .SelfClosing = true})}}))

  TEST("AttributeName", "ignores whitespace and switches to AfterAttributeName when parsing whitespace",
       (UnitTest {.ExpectedState = TokenizerState::AfterAttributeName, .Input = U"<div b   \t\n\r"}))

  TEST("AttributeName", "switches to SelfClosingStartTag when parsing Solidus",
       (UnitTest {.ExpectedState = TokenizerState::SelfClosingStartTag, .Input = U"<div b/"}))

  TEST("AttributeName", "parses attribute name and switches to BeforeAttributeValue when parsing EqualsSign",
       (UnitTest {.ExpectedState = TokenizerState::BeforeAttributeValue, .Input = U"<div b="}))

  TEST("AttributeName", "appends to attribute name when parsing valid attribute name characters",
       (UnitTest {.ExpectedState = TokenizerState::BeforeAttributeValue, .Input = U"<div data-value="}))

  TEST("AttributeName", "Replaces null character with U+FFFD in attribute name",
       (UnitTest {
         .ExpectedState = TokenizerState::BeforeAttributeValue,
         .Input = InsertUTF32Null(U"<div data", U"value="),
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 10uz}}}))

  TEST("AttributeName", "Treats QuotationMark as anything else but with parse error",
       (UnitTest {
         .ExpectedState = TokenizerState::BeforeAttributeValue,
         .Input = U"<div data\"value=",
         .Errors = {
           {.Error = HTMLParseError::UnexpectedCharacterInAttributeName, .Line = 1uz, .Column = 10uz}}}))

  TEST("AttributeName", "Treats Apostrophe as anything else but with parse error",
       (UnitTest {
         .ExpectedState = TokenizerState::BeforeAttributeValue,
         .Input = U"<div data'value=",
         .Errors = {
           {.Error = HTMLParseError::UnexpectedCharacterInAttributeName, .Line = 1uz, .Column = 10uz}}}))

  TEST("AttributeName", "Treats LessThanSign as anything else but with parse error",
       (UnitTest {
         .ExpectedState = TokenizerState::BeforeAttributeValue,
         .Input = U"<div data<value=",
         .Errors = {
           {.Error = HTMLParseError::UnexpectedCharacterInAttributeName, .Line = 1uz, .Column = 10uz}}}))

#pragma endregion

#pragma region AfterAttributeName

  TEST("AfterAttributeName", "ignores whitespace and stays in the same state when parsing whitespace",
       (UnitTest {.InitialState = TokenizerState::AfterAttributeName,
                  .ExpectedState = TokenizerState::AfterAttributeName,
                  .Input = U"   \t\n\r"}))

  TEST("AfterAttributeName", "switches to SelfClosingStartTag when parsing Solidus",
       (UnitTest {.InitialState = TokenizerState::AfterAttributeName,
                  .ExpectedState = TokenizerState::SelfClosingStartTag,
                  .Input = U"/"}))

  TEST("AfterAttributeName", "switches to BeforeAttributeValue after parsing an EqualsSign",
       (UnitTest {.InitialState = TokenizerState::Data,
                  .ExpectedState = TokenizerState::BeforeAttributeValue,
                  .Input = U"<a b="}))

  TEST("AfterAttributeName", "switches to Data and emits the current tag when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<div b>",
         .Output = {CreateStartTagToken({.Name = u8"div", .Attributes = {{.Name = u8"b", .Value = u8""}}})}}))

  TEST("AfterAttributeName", "Emits EOF token if EOF is reached",
       (UnitTest {.Input = U"<div b ",
                  .AppendEOF = true,
                  .Output = {CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInTag, .Line = 1uz, .Column = 8uz}}}))

  TEST("AfterAttributeName", "treats any other character as the start of a new attribute name",
       (UnitTest {.ExpectedState = TokenizerState::AttributeName, .Input = U"<div a b"}))

#pragma endregion

#pragma region BeforeAttributeValue

  TEST("BeforeAttributeValue", "ignores whitespace and stays in the same state when parsing whitespace",
       (UnitTest {.InitialState = TokenizerState::BeforeAttributeValue,
                  .ExpectedState = TokenizerState::BeforeAttributeValue,
                  .Input = U"   \t\n\r"}))

  TEST("BeforeAttributeValue", "switches to AttributeValueDoubleQuoted when parsing QuotationMark",
       (UnitTest {.ExpectedState = TokenizerState::AttributeValueDoubleQuoted, .Input = U"<div a=\""}))

  TEST("BeforeAttributeValue", "switches to AttributeValueSingleQuoted when parsing Apostrophe",
       (UnitTest {.ExpectedState = TokenizerState::AttributeValueSingleQuoted, .Input = U"<div a='"}))

  TEST("BeforeAttributeValue",
       "emits current tag token and switches to Data with parser error when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<div a=>",
         .Output = {CreateStartTagToken({.Name = u8"div", .Attributes = {{.Name = u8"a", .Value = u8""}}})},
         .Errors = {{.Error = HTMLParseError::MissingAttributeValue, .Line = 1uz, .Column = 8uz}}}))

  TEST("BeforeAttributeValue", "switches to AttributeValueUnquoted when parsing any other character",
       (UnitTest {.ExpectedState = TokenizerState::AttributeValueUnquoted, .Input = U"<div a=value"}))

  TEST("BeforeAttributeValue", "emits EOF token if EOF is reached",
       (UnitTest {.Input = U"<div a=",
                  .AppendEOF = true,
                  .Output = {CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInTag, .Line = 1uz, .Column = 8uz}}}))

#pragma endregion

#pragma region AttributeValueDoubleQuoted

  TEST("AttributeValueDoubleQuoted", "switches to AfterAttributeValueQuoted when parsing QuotationMark ",
       (UnitTest {.ExpectedState = TokenizerState::AfterAttributeValueQuoted, .Input = U"<div a=\"value\""}))

  TEST("AttributeValueDoubleQuoted", "switches to CharacterReference when parsing Ampersand ",
       (UnitTest {.ExpectedState = TokenizerState::CharacterReference, .Input = U"<div a=\"value&"}))

  TEST("AttributeValueDoubleQuoted", "Replaces null character with U+FFFD",
       (UnitTest {
         .Input = InsertUTF32Null(U"<div a=\"value", U"\">"),
         .Output = {CreateStartTagToken({.Name = u8"div",
                                         .Attributes = {{.Name = u8"a", .Value = u8"value\uFFFD"}}})},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 14uz}}}))

  TEST("AttributeValueDoubleQuoted", "Emits EOF instead of tag if EOF reached",
       (UnitTest {.Input = U"<div a=\"value",
                  .AppendEOF = true,
                  .Output = {CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInTag, .Line = 1uz, .Column = 14uz}}}))

  TEST(
    "AttributeValueDoubleQuoted", "appends to attribute value when parsing any other character",
    (UnitTest {
      .Input = U"<div a=\"valúe\">",
      .Output = {CreateStartTagToken({.Name = u8"div", .Attributes = {{.Name = u8"a", .Value = u8"valúe"}}})},
    }))

#pragma endregion

#pragma region AttributeValueSingleQuoted

  TEST("AttributeValueSingleQuoted", "switches to AfterAttributeValueQuoted when parsing Apostrophe ",
       (UnitTest {.ExpectedState = TokenizerState::AfterAttributeValueQuoted, .Input = U"<div a='value'"}))

  TEST("AttributeValueSingleQuoted", "switches to CharacterReference when parsing Ampersand ",
       (UnitTest {.ExpectedState = TokenizerState::CharacterReference, .Input = U"<div a='value&"}))

  TEST("AttributeValueSingleQuoted", "Replaces null character with U+FFFD",
       (UnitTest {
         .Input = InsertUTF32Null(U"<div a='value", U"'>"),
         .Output = {CreateStartTagToken({.Name = u8"div",
                                         .Attributes = {{.Name = u8"a", .Value = u8"value\uFFFD"}}})},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 14uz}}}))

  TEST("AttributeValueSingleQuoted", "Emits EOF instead of tag if EOF reached",
       (UnitTest {.Input = U"<div a='value",
                  .AppendEOF = true,
                  .Output = {CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInTag, .Line = 1uz, .Column = 14uz}}}))

  TEST(
    "AttributeValueSingleQuoted", "appends to attribute value when parsing any other character",
    (UnitTest {
      .Input = U"<div a='valúe'>",
      .Output = {CreateStartTagToken({.Name = u8"div", .Attributes = {{.Name = u8"a", .Value = u8"valúe"}}})},
    }))

#pragma endregion

#pragma region AttributeValueUnquoted

  TEST(
    "AttributeValueUnquoted", "switches to Data and emits the current tag when parsing GreaterThanSign",
    (UnitTest {
      .Input = U"<div a=value>",
      .Output = {CreateStartTagToken({.Name = u8"div", .Attributes = {{.Name = u8"a", .Value = u8"value"}}})},
    }))

  TEST("AttributeValueUnquoted", "Replaces null character with U+FFFD",
       (UnitTest {
         .Input = InsertUTF32Null(U"<div a=value", U">"),
         .Output = {CreateStartTagToken({.Name = u8"div",
                                         .Attributes = {{.Name = u8"a", .Value = u8"value\uFFFD"}}})},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 13uz}}}))

  TEST("AttributeValueUnquoted", "treats QuotationMark as anything else but with parse error",
       (UnitTest {.Input = U"<div a=val\"ue>",
                  .Output = {CreateStartTagToken({.Name = u8"div",
                                                  .Attributes = {{.Name = u8"a", .Value = u8"val\"ue"}}})},
                  .Errors = {{.Error = HTMLParseError::UnexpectedCharacterInUnquotedAttributeValue,
                              .Line = 1uz,
                              .Column = 11uz}}}))

  TEST("AttributeValueUnquoted", "treats Apostrophe as anything else but with parse error",
       (UnitTest {.Input = U"<div a=val'ue>",
                  .Output = {CreateStartTagToken({.Name = u8"div",
                                                  .Attributes = {{.Name = u8"a", .Value = u8"val'ue"}}})},
                  .Errors = {{.Error = HTMLParseError::UnexpectedCharacterInUnquotedAttributeValue,
                              .Line = 1uz,
                              .Column = 11uz}}}))

  TEST("AttributeValueUnquoted", "treats LessThanSign as anything else but with parse error",
       (UnitTest {.Input = U"<div a=val<ue>",
                  .Output = {CreateStartTagToken({.Name = u8"div",
                                                  .Attributes = {{.Name = u8"a", .Value = u8"val<ue"}}})},
                  .Errors = {{.Error = HTMLParseError::UnexpectedCharacterInUnquotedAttributeValue,
                              .Line = 1uz,
                              .Column = 11uz}}}))

  TEST("AttributeValueUnquoted", "treats EqualsSign as anything else but with parse error",
       (UnitTest {.Input = U"<div a=val=ue>",
                  .Output = {CreateStartTagToken({.Name = u8"div",
                                                  .Attributes = {{.Name = u8"a", .Value = u8"val=ue"}}})},
                  .Errors = {{.Error = HTMLParseError::UnexpectedCharacterInUnquotedAttributeValue,
                              .Line = 1uz,
                              .Column = 11uz}}}))

  TEST("AttributeValueUnquoted", "treats GraveAccent as anything else but with parse error",
       (UnitTest {.Input = U"<div a=val`ue>",
                  .Output = {CreateStartTagToken({.Name = u8"div",
                                                  .Attributes = {{.Name = u8"a", .Value = u8"val`ue"}}})},
                  .Errors = {{.Error = HTMLParseError::UnexpectedCharacterInUnquotedAttributeValue,
                              .Line = 1uz,
                              .Column = 11uz}}}))

  TEST("AttributeValueUnquoted", "switches to BeforeAttributeName when parsing whitespace",
       (UnitTest {.ExpectedState = TokenizerState::BeforeAttributeName, .Input = U"<div a=value "}))

  TEST("AttributeValueUnquoted", "switches to CharacterReference when parsing Ampersand",
       (UnitTest {.ExpectedState = TokenizerState::CharacterReference, .Input = U"<div a=value&"}))

  TEST("AttributeValueUnquoted", "Emits EOF instead of tag if EOF reached",
       (UnitTest {.Input = U"<div a=value",
                  .AppendEOF = true,
                  .Output = {CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInTag, .Line = 1uz, .Column = 13uz}}}))

#pragma endregion

#pragma region AfterAttributeValueQuoted

  TEST("AfterAttributeValueQuoted", "switches to Data and emits the current tag when parsing GreaterThanSign",
       (UnitTest {.Input = U"<div a=\"value\">",
                  .Output = {CreateStartTagToken({.Name = u8"div",
                                                  .Attributes = {{.Name = u8"a", .Value = u8"value"}}})}}))

  TEST("AfterAttributeValueQuoted",
       "ignores whitespace and switches to BeforeAttributeName when parsing whitespace",
       (UnitTest {.ExpectedState = TokenizerState::BeforeAttributeName,
                  .Input = U"<div a=\"value\"   \t\n\r"}))

  TEST("AfterAttributeValueQuoted", "switches to SelfClosingStartTag when parsing Solidus",
       (UnitTest {.ExpectedState = TokenizerState::SelfClosingStartTag, .Input = U"<div a=\"value\"/"}))

  TEST("AfterAttributeValueQuoted", "Emits EOF instead of tag if EOF reached",
       (UnitTest {.Input = U"<div a=\"value\"",
                  .AppendEOF = true,
                  .Output = {CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInTag, .Line = 1uz, .Column = 15uz}}}))

  TEST("AfterAttributeValueQuoted",
       "Treats anything else as missing whitespace and switches to AttributeName",
       (UnitTest {
         .ExpectedState = TokenizerState::AttributeName,
         .Input = U"<div a=\"value\"b",
         .Errors = {
           {.Error = HTMLParseError::MissingWhitespaceBetweenAttributes, .Line = 1uz, .Column = 15uz}}}))

#pragma endregion

#pragma region SelfClosingStartTag

  TEST("SelfClosingStartTag",
       "switches to Data and emits the current tag as self-closing when parsing GreaterThanSign",
       (UnitTest {.Input = U"<div />",
                  .Output = {CreateStartTagToken({.Name = u8"div", .SelfClosing = true})}}))

  TEST("SelfClosingStartTag", "Emits EOF instead of tag if EOF reached",
       (UnitTest {.Input = U"<div /",
                  .AppendEOF = true,
                  .Output = {CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInTag, .Line = 1uz, .Column = 7uz}}}))

  TEST("SelfClosingStartTag",
       "Treats anything else as unexpected Solidus with parser error and switches to AttributeName",
       (UnitTest {.ExpectedState = TokenizerState::AttributeName,
                  .Input = U"<div /a",
                  .Errors = {{.Error = HTMLParseError::UnexpectedSolidusInTag, .Line = 1uz, .Column = 7uz}}}))

#pragma endregion

#pragma region BogusComment

  TEST("BogusComment", "emits comment and switches to Data when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<?comment>",
         .Output = {CreateCommentToken(u8"?comment")},
         .Errors = {
           {.Error = HTMLParseError::UnexpectedQuestionMarkInsteadOfTagName, .Line = 1uz, .Column = 2uz}}}))

  TEST("BogusComment", "emits comment and then EOF when EOF reached",
       (UnitTest {
         .Input = U"<?comment",
         .AppendEOF = true,
         .Output = {CreateCommentToken(u8"?comment"), CreateEOFToken()},
         .Errors = {
           {.Error = HTMLParseError::UnexpectedQuestionMarkInsteadOfTagName, .Line = 1uz, .Column = 2uz}}}))

  TEST("BogusComment", "appends to comment when parsing NullCharacter",
       (UnitTest {
         .Input = InsertUTF32Null(U"<?com", U"ment>"),
         .Output = {CreateCommentToken(u8"?com\uFFFDment")},
         .Errors = {
           {.Error = HTMLParseError::UnexpectedQuestionMarkInsteadOfTagName, .Line = 1uz, .Column = 2uz},
           {.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 6uz}}}))

  TEST("BogusComment", "appends to comment when parsing any other character",
       (UnitTest {
         .Input = U"<?comm>",
         .Output = {CreateCommentToken(u8"?comm")},
         .Errors = {
           {.Error = HTMLParseError::UnexpectedQuestionMarkInsteadOfTagName, .Line = 1uz, .Column = 2uz}}}))

#pragma endregion

#pragma region MarkupDeclarationOpen

  TEST("MarkupDeclarationOpen", "switches to CommentStart when parsing DoubleHyphen",
       (UnitTest {.ExpectedState = TokenizerState::CommentStart, .Input = U"<!--"}))

  TEST("MarkupDeclarationOpen", "switches to DOCTYPE when parsing 'DOCTYPE'",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPE, .Input = U"<!DOCTYPE"}))

  TEST("MarkupDeclarationOpen", "switches to DOCTYPE when parsing 'DOCTYPE', mixed case",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPE, .Input = U"<!DoCtYpe"}))

  TEST("MarkupDeclarationOpen", "switches to BogusComment with parser error when parsing anything else",
       (UnitTest {
         .ExpectedState = TokenizerState::BogusComment,
         .Input = U"<!comment",
         .Errors = {{.Error = HTMLParseError::IncorrectlyOpenedComment, .Line = 1uz, .Column = 3uz}}}))

  TEST("MarkupDeclarationOpen", "Switches to CDATASection when parsing '[CDATA[' and CDATA is allowed",
       (UnitTest {
         .ExpectedState = TokenizerState::CDATASection, .Input = U"<![CDATA[", .CDATASectionAllowed = true}))

  TEST("MarkupDeclarationOpen", "Switches to BogusComment when parsing '[CDATA[' and CDATA is not allowed",
       (UnitTest {.ExpectedState = TokenizerState::BogusComment,
                  .Input = U"<![CDATA[",
                  .Errors = {{.Error = HTMLParseError::CDATAInHTMLContent, .Line = 1uz, .Column = 10uz}}}))

#pragma endregion

#pragma region CommentStart

  TEST("CommentStart", "switches to CommentStartDash when parsing HyphenMinus",
       (UnitTest {.ExpectedState = TokenizerState::CommentStartDash, .Input = U"<!---"}))

  TEST("CommentStart", "switches to Data and emits comment with parser error when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<!-->",
         .Output = {CreateCommentToken(u8"")},
         .Errors = {{.Error = HTMLParseError::AbruptClosingOfEmptyComment, .Line = 1uz, .Column = 5uz}}}))

  TEST("CommentStart", "reconsumes in Comment for anything else",
       (UnitTest {.ExpectedState = TokenizerState::Comment, .Input = U"<!--c"}))

#pragma endregion

#pragma region CommentStartDash

  TEST("CommentStartDash", "switches to CommentEnd when parsing HyphenMinus",
       (UnitTest {.ExpectedState = TokenizerState::CommentEnd, .Input = U"<!----"}))

  TEST("CommentStartDash",
       "switches to Data and emits comment with parser error when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<!--->",
         .Output = {CreateCommentToken(u8"")},
         .Errors = {{.Error = HTMLParseError::AbruptClosingOfEmptyComment, .Line = 1uz, .Column = 6uz}}}))

  TEST("CommentStartDash", "Emits comment and then EOF when EOF reached",
       (UnitTest {.Input = U"<!---comment",
                  .AppendEOF = true,
                  .Output = {CreateCommentToken(u8"-comment"), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInComment, .Line = 1uz, .Column = 13uz}}}))

  TEST("CommentStartDash", "reconsumes in Comment for anything else",
       (UnitTest {.ExpectedState = TokenizerState::Comment, .Input = U"<!---c"}))

#pragma endregion

#pragma region Comment

  TEST("Comment", "switches to CommentLessThanSign when parsing LessThanSign",
       (UnitTest {.ExpectedState = TokenizerState::CommentLessThanSign, .Input = U"<!--a<"}))

  TEST("Comment", "switches to CommentEndDash when parsing HyphenMinus",
       (UnitTest {.ExpectedState = TokenizerState::CommentEndDash, .Input = U"<!--comment-"}))

  TEST("Comment", "Replaces null character with U+FFFD",
       (UnitTest {
         .Input = InsertUTF32Null(U"<!--com", U"ment-->"),
         .Output = {CreateCommentToken(u8"com\uFFFDment")},
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 8uz}}}))

  TEST("Comment", "emits comment and then EOF when EOF reached",
       (UnitTest {.Input = U"<!--comment",
                  .AppendEOF = true,
                  .Output = {CreateCommentToken(u8"comment"), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInComment, .Line = 1uz, .Column = 12uz}}}))

  TEST("Comment", "appends to comment when parsing any other character",
       (UnitTest {.ExpectedState = TokenizerState::Comment, .Input = U"<!--comment"}))

#pragma endregion

#pragma region CommentLessThanSign

  TEST("CommentLessThanSign", "witches to CommentLessThanSignBang when parsing ExclamationMark",
       (UnitTest {.ExpectedState = TokenizerState::CommentLessThanSignBang, .Input = U"<!--a<!"}))

  TEST("CommentLessThanSign",
       "appends LessThanSign and remains in CommentLessThanSign when parsing LessThanSign",
       (UnitTest {.ExpectedState = TokenizerState::CommentLessThanSign, .Input = U"<!--a<<"}))

  TEST("CommentLessThanSign", "reconsumes in Comment for anything else",
       (UnitTest {.ExpectedState = TokenizerState::Comment, .Input = U"<!--a<b"}))

#pragma endregion

#pragma region CommentLessThanSignBang

  TEST("CommentLessThanSignBang", "switches to CommentLessThanSignBangDash when parsing HyphenMinus",
       (UnitTest {.ExpectedState = TokenizerState::CommentLessThanSignBangDash, .Input = U"<!--a<!-"}))

  TEST("CommentLessThanSignBang", "reconsumes in Comment for anything else",
       (UnitTest {.ExpectedState = TokenizerState::Comment, .Input = U"<!--a<!b"}))

#pragma endregion

#pragma region CommentLessThanSignBangDash

  TEST("CommentLessThanSignBangDash", "switches to CommentLessThanSignBangDashDash when parsing HyphenMinus",
       (UnitTest {.ExpectedState = TokenizerState::CommentLessThanSignBangDashDash, .Input = U"<!--a<!--"}))

  TEST("CommentLessThanSignBangDash", "reconsumes in Comment for anything else",
       (UnitTest {.ExpectedState = TokenizerState::Comment, .Input = U"<!--a<!-b"}))

#pragma endregion

#pragma region CommentLessThanSignBangDashDash

  TEST("CommentLessThanSignBangDashDash", "emits comment when parsing GreaterThanSign",
       (UnitTest {.Input = U"<!--a<!-->", .Output = {CreateCommentToken(u8"a<!")}}))

  TEST("CommentLessThanSignBangDashDash", "emits comment with parser error followed by EOF when parsing EOF",
       (UnitTest {.ExpectedState = TokenizerState::Data,
                  .Input = U"<!--a<!--",
                  .AppendEOF = true,
                  .Output = {CreateCommentToken(u8"a<!"), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInComment, .Line = 1uz, .Column = 10uz}}}))

  TEST("CommentLessThanSignBangDashDash", "reconsumes in Comment for anything else",
       (UnitTest {.ExpectedState = TokenizerState::Comment,
                  .Input = U"<!--a<!--b",
                  .Errors = {{.Error = HTMLParseError::NestedComment, .Line = 1uz, .Column = 10uz}}}))

#pragma endregion

#pragma region CommentEndDash

  TEST("CommentEndDash", "switches to CommentEnd when parsing HyphenMinus",
       (UnitTest {.ExpectedState = TokenizerState::CommentEnd, .Input = U"<!--com--"}))

  TEST("CommentEndDash", "emits comment with parser error followed by EOF when parsing EOF",
       (UnitTest {.Input = U"<!--com-",
                  .AppendEOF = true,
                  .Output = {CreateCommentToken(u8"com"), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInComment, .Line = 1uz, .Column = 9uz}}}))

  TEST("CommentEndDash", "reconsumes in Comment for anything else",
       (UnitTest {.ExpectedState = TokenizerState::Comment, .Input = U"<!--com-a"}))

#pragma endregion

#pragma region CommentEnd

  TEST("CommentEnd", "emits comment when parsing GreaterThanSign",
       (UnitTest {.Input = U"<!--comment-->", .Output = {CreateCommentToken(u8"comment")}}))

  TEST("CommentEnd", "switches to CommentEndBang when parsing Exclamation",
       (UnitTest {.ExpectedState = TokenizerState::CommentEndBang, .Input = U"<!--comment--!"}))

  TEST("CommentEnd", "remains in CommentEnd when parsing HyphenMinus",
       (UnitTest {.ExpectedState = TokenizerState::CommentEnd, .Input = U"<!--comment---"}))

  TEST("CommentEnd", "emits comment with parser error followed by EOF when parsing EOF",
       (UnitTest {.Input = U"<!--comment--",
                  .AppendEOF = true,
                  .Output = {CreateCommentToken(u8"comment"), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInComment, .Line = 1uz, .Column = 14uz}}}))

  TEST("CommentEnd", "reconsumes in Comment with parser error for anything else",
       (UnitTest {.ExpectedState = TokenizerState::Comment, .Input = U"<!--comment--a"}))

#pragma endregion

#pragma region CommentEndBang

  TEST("CommentEndBang", "switches to CommentEndDash when parsing HyphenMinus",
       (UnitTest {.ExpectedState = TokenizerState::CommentEndDash, .Input = U"<!--comment--!-"}))

  TEST("CommentEndBang", "emits comment when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<!--comment--!>",
         .Output = {CreateCommentToken(u8"comment")},
         .Errors = {{.Error = HTMLParseError::IncorrectlyClosedComment, .Line = 1uz, .Column = 15uz}}}))

  TEST("CommentEndBang", "emits comment with parser error followed by EOF when parsing EOF",
       (UnitTest {.Input = U"<!--comment--!",
                  .AppendEOF = true,
                  .Output = {CreateCommentToken(u8"comment"), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInComment, .Line = 1uz, .Column = 15uz}}}))

  TEST("CommentEndBang", "reconsumes in Comment for anything else",
       (UnitTest {.ExpectedState = TokenizerState::Comment, .Input = U"<!--comment--!a"}))

#pragma endregion

#pragma region DOCTYPE

  TEST("DOCTYPE", "ignores whitespace and switches to BeforeDOCTYPEName when parsing whitespace",
       (UnitTest {.ExpectedState = TokenizerState::BeforeDOCTYPEName, .Input = U"<!DOCTYPE   \t\n\r"}))

  TEST("DOCTYPE", "emits DOCTYPE token with force-quirks when parsing GreaterThanSign",
       (UnitTest {.Input = U"<!DOCTYPE>",
                  .Output = {CreateDOCTYPEToken({.ForceQuirks = true})},
                  .Errors = {{.Error = HTMLParseError::MissingDOCTYPEName, .Line = 1uz, .Column = 10uz}}}))

  TEST("DOCTYPE", "emits DOCTYPE token with force-quirks when EOF reached",
       (UnitTest {.Input = U"<!DOCTYPE",
                  .AppendEOF = true,
                  .Output = {CreateDOCTYPEToken({.ForceQuirks = true}), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInDOCTYPE, .Line = 1uz, .Column = 10uz}}}))

  TEST("DOCTYPE", "treats anything else as missing whitespace before DOCTYPE name",
       (UnitTest {
         .ExpectedState = TokenizerState::DOCTYPEName,
         .Input = U"<!DOCTYPEa",
         .Errors = {
           {.Error = HTMLParseError::MissingWhitespaceBeforeDOCTYPEName, .Line = 1uz, .Column = 10uz}}}))

#pragma endregion

#pragma region BeforeDOCTYPEName

  TEST("BeforeDOCTYPEName", "ignores whitespace and stays in the same state when parsing whitespace",
       (UnitTest {.InitialState = TokenizerState::BeforeDOCTYPEName,
                  .ExpectedState = TokenizerState::BeforeDOCTYPEName,
                  .Input = U"   \t\n\r"}))

  TEST("BeforeDOCTYPEName", "switches to DOCTYPEName when parsing ASCII lower",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPEName, .Input = U"<!DOCTYPE a"}))

  TEST("BeforeDOCTYPEName", "switches to DOCTYPEName when parsing ASCII upper",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPEName, .Input = U"<!DOCTYPE A"}))

  TEST("BeforeDOCTYPEName", "emits ReplacementCharacter with parser error when parsing Null",
       (UnitTest {
         .ExpectedState = TokenizerState::DOCTYPEName,
         .Input = InsertUTF32Null(U"<!DOCTYPE", U"a"),
         .Errors = {
           {.Error = HTMLParseError::MissingWhitespaceBeforeDOCTYPEName, .Line = 1uz, .Column = 10uz},
           {.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 10uz}}}))

  TEST("BeforeDOCTYPEName", "emits DOCTYPE with force-quirks when parsing GreaterThanSign",
       (UnitTest {.Input = U"<!DOCTYPE>",
                  .Output = {CreateDOCTYPEToken({.ForceQuirks = true})},
                  .Errors = {{.Error = HTMLParseError::MissingDOCTYPEName, .Line = 1uz, .Column = 10uz}}}))

  TEST("BeforeDOCTYPEName", "emits DOCTYPE with force-quirks when EOF reached",
       (UnitTest {.Input = U"<!DOCTYPE",
                  .AppendEOF = true,
                  .Output = {CreateDOCTYPEToken({.ForceQuirks = true}), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInDOCTYPE, .Line = 1uz, .Column = 10uz}}}))

  TEST("BeforeDOCTYPEName", "treats anything else as start of DOCTYPE name",
       (UnitTest {
         .ExpectedState = TokenizerState::DOCTYPEName,
         .Input = U"<!DOCTYPEa",
         .Errors = {
           {.Error = HTMLParseError::MissingWhitespaceBeforeDOCTYPEName, .Line = 1uz, .Column = 10uz}}}))

#pragma endregion

#pragma region DOCTYPEName

  TEST("DOCTYPEName", "ignores whitespace and switches to AfterDOCTYPEName when parsing whitespace",
       (UnitTest {.ExpectedState = TokenizerState::AfterDOCTYPEName, .Input = U"<!DOCTYPE HTML5 "}))

  TEST("DOCTYPEName", "switches to Data and emits DOCTYPE when parsing GreaterThanSign",
       (UnitTest {.Input = U"<!DOCTYPE HTML5>", .Output = {CreateDOCTYPEToken({.Name = u8"html5"})}}))

  TEST("DOCTYPEName", "appends to name when parsing ASCII lower",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPEName, .Input = U"<!DOCTYPE html5"}))

  TEST("DOCTYPEName", "appends to name when parsing ASCII upper",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPEName, .Input = U"<!DOCTYPE HTML5"}))

  TEST("DOCTYPEName", "appends ReplacementCharacter with parser error when parsing Null",
       (UnitTest {
         .ExpectedState = TokenizerState::DOCTYPEName,
         .Input = InsertUTF32Null(U"<!DOCTYPE ht", U"ml5"),
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 13uz}}}))

  TEST("DOCTYPEName", "emits DOCTYPE with force-quirks when EOF reached",
       (UnitTest {.Input = U"<!DOCTYPE HTML5",
                  .AppendEOF = true,
                  .Output = {CreateDOCTYPEToken({.Name = u8"html5", .ForceQuirks = true}), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInDOCTYPE, .Line = 1uz, .Column = 16uz}}}))

  TEST("DOCTYPEName", "treats anything else as part of DOCTYPE name",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPEName, .Input = U"<!DOCTYPE HTML5a"}))

#pragma endregion

#pragma region AfterDOCTYPEName

  TEST("AfterDOCTYPEName", "ignores whitespace and stays in the same state when parsing whitespace",
       (UnitTest {.InitialState = TokenizerState::AfterDOCTYPEName,
                  .ExpectedState = TokenizerState::AfterDOCTYPEName,
                  .Input = U"   \t\n\r"}))

  TEST("AfterDOCTYPEName", "switches to Data and emits DOCTYPE when parsing GreaterThanSign",
       (UnitTest {.Input = U"<!DOCTYPE HTML5>", .Output = {CreateDOCTYPEToken({.Name = u8"html5"})}}))

  TEST("AfterDOCTYPEName", "emits DOCTYPE with force-quirks when EOF reached",
       (UnitTest {.Input = U"<!DOCTYPE HTML5",
                  .AppendEOF = true,
                  .Output = {CreateDOCTYPEToken({.Name = u8"html5", .ForceQuirks = true}), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInDOCTYPE, .Line = 1uz, .Column = 16uz}}}))

  TEST("DOCTYPEName", "switches to AfterDOCTYPEPublicKeyword when parsing 'PUBLIC'",
       (UnitTest {.ExpectedState = TokenizerState::AfterDOCTYPEPublicKeyword,
                  .Input = U"<!DOCTYPE HTML PUBLIC"}))

  TEST("DOCTYPEName", "switches to AfterDOCTYPESystemKeyword when parsing 'SYSTEM'",
       (UnitTest {.ExpectedState = TokenizerState::AfterDOCTYPESystemKeyword,
                  .Input = U"<!DOCTYPE HTML SYSTEM"}))

  TEST("DOCTYPEName", "treats anything else as unexpected and switches to BogusDOCTYPE",
       (UnitTest {.ExpectedState = TokenizerState::BogusDOCTYPE,
                  .Input = U"<!DOCTYPE HTML a",
                  .Errors = {{.Error = HTMLParseError::InvalidCharacterSequenceAfterDOCTYPEName,
                              .Line = 1uz,
                              .Column = 16uz}}}))

#pragma endregion

#pragma region AfterDOCTYPEPublicKeyword

  TEST("AfterDOCTYPEPublicKeyword",
       "ignores whitespace and switches to BeforeDOCTYPEPublicIdentifier when parsing whitespace",
       (UnitTest {.ExpectedState = TokenizerState::BeforeDOCTYPEPublicIdentifier,
                  .Input = U"<!DOCTYPE HTML PUBLIC   \t\n\r"}))

  TEST("AfterDOCTYPEPublicKeyword",
       "switches to DOCTYPEPublicIdentifierDoubleQuoted with parser error when parsing QuotationMark",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPEPublicIdentifierDoubleQuoted,
                  .Input = U"<!DOCTYPE HTML PUBLIC\"",
                  .Errors = {{.Error = HTMLParseError::MissingWhitespaceAfterDOCTYPEPublicKeyword,
                              .Line = 1uz,
                              .Column = 22uz}}}))

  TEST("AfterDOCTYPEPublicKeyword",
       "switches to DOCTYPEPublicIdentifierSingleQuoted with parser error when parsing Apostrophe",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPEPublicIdentifierSingleQuoted,
                  .Input = U"<!DOCTYPE HTML PUBLIC'",
                  .Errors = {{.Error = HTMLParseError::MissingWhitespaceAfterDOCTYPEPublicKeyword,
                              .Line = 1uz,
                              .Column = 22uz}}}))

  TEST("AfterDOCTYPEPublicKeyword", "emits DOCTYPE with force-quirks when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<!DOCTYPE HTML PUBLIC>",
         .Output = {CreateDOCTYPEToken({.Name = u8"html", .ForceQuirks = true})},
         .Errors = {{.Error = HTMLParseError::MissingDOCTYPEPublicIdentifier, .Line = 1uz, .Column = 22uz}}}))

  TEST("AfterDOCTYPEPublicKeyword", "emits DOCTYPE with force-quirks when EOF reached",
       (UnitTest {.Input = U"<!DOCTYPE HTML PUBLIC",
                  .AppendEOF = true,
                  .Output = {CreateDOCTYPEToken({.Name = u8"html", .ForceQuirks = true}), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInDOCTYPE, .Line = 1uz, .Column = 22uz}}}))

  TEST("AfterDOCTYPEPublicKeyword", "treats anything else as unexpected and switches to BogusDOCTYPE",
       (UnitTest {.ExpectedState = TokenizerState::BogusDOCTYPE,
                  .Input = U"<!DOCTYPE HTML PUBLICa",
                  .Errors = {{.Error = HTMLParseError::MissingQuoteBeforeDOCTYPEPublicIdentifier,
                              .Line = 1uz,
                              .Column = 22uz}}}))

#pragma endregion

#pragma region BeforeDOCTYPEPublicIdentifier

  TEST("BeforeDOCTYPEPublicIdentifier",
       "ignores whitespace and stays in the same state when parsing whitespace",
       (UnitTest {.InitialState = TokenizerState::BeforeDOCTYPEPublicIdentifier,
                  .ExpectedState = TokenizerState::BeforeDOCTYPEPublicIdentifier,
                  .Input = U"   \t\n\r"}))

  TEST("BeforeDOCTYPEPublicIdentifier",
       "switches to DOCTYPEPublicIdentifierDoubleQuoted when parsing QuotationMark",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPEPublicIdentifierDoubleQuoted,
                  .Input = U"<!DOCTYPE HTML PUBLIC \""}))

  TEST("BeforeDOCTYPEPublicIdentifier",
       "switches to DOCTYPEPublicIdentifierSingleQuoted when parsing Apostrophe",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPEPublicIdentifierSingleQuoted,
                  .Input = U"<!DOCTYPE HTML PUBLIC '"}))

  TEST("BeforeDOCTYPEPublicIdentifier", "emits DOCTYPE with force-quirks when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<!DOCTYPE HTML PUBLIC>",
         .Output = {CreateDOCTYPEToken({.Name = u8"html", .ForceQuirks = true})},
         .Errors = {{.Error = HTMLParseError::MissingDOCTYPEPublicIdentifier, .Line = 1uz, .Column = 22uz}}}))

  TEST("BeforeDOCTYPEPublicIdentifier", "emits DOCTYPE with force-quirks when EOF reached",
       (UnitTest {.Input = U"<!DOCTYPE HTML PUBLIC",
                  .AppendEOF = true,
                  .Output = {CreateDOCTYPEToken({.Name = u8"html", .ForceQuirks = true}), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInDOCTYPE, .Line = 1uz, .Column = 22uz}}}))

  TEST("BeforeDOCTYPEPublicIdentifier", "treats anything else as unexpected and switches to BogusDOCTYPE",
       (UnitTest {.ExpectedState = TokenizerState::BogusDOCTYPE,
                  .Input = U"<!DOCTYPE HTML PUBLICa",
                  .Errors = {{.Error = HTMLParseError::MissingQuoteBeforeDOCTYPEPublicIdentifier,
                              .Line = 1uz,
                              .Column = 22uz}}}))

#pragma endregion

#pragma region DOCTYPEPublicIdentifierDoubleQuoted

  TEST("DOCTYPEPublicIdentifierDoubleQuoted",
       "switches to AfterDOCTYPEPublicIdentifier when parsing QuotationMark",
       (UnitTest {.ExpectedState = TokenizerState::AfterDOCTYPEPublicIdentifier,
                  .Input = U"<!DOCTYPE HTML PUBLIC \"identifier\""}))

  TEST("DOCTYPEPublicIdentifierDoubleQuoted", "replaces Null with U+FFFD",
       (UnitTest {
         .ExpectedState = TokenizerState::DOCTYPEPublicIdentifierDoubleQuoted,
         .Input = InsertUTF32Null(U"<!DOCTYPE HTML PUBLIC \"iden", U"tifier"),
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 28uz}}}))

  TEST("DOCTYPEPublicIdentifierDoubleQuoted", "emits DOCTYPE with force-quirks when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<!DOCTYPE HTML PUBLIC \"identifier>",
         .Output = {CreateDOCTYPEToken(
           {.Name = u8"html", .PublicIdentifier = u8"identifier", .ForceQuirks = true})},
         .Errors = {{.Error = HTMLParseError::AbruptDOCTYPEPublicIdentifier, .Line = 1uz, .Column = 34uz}}}))

  TEST("DOCTYPEPublicIdentifierDoubleQuoted", "emits DOCTYPE with force-quirks when EOF reached",
       (UnitTest {.Input = U"<!DOCTYPE HTML PUBLIC \"identifier",
                  .AppendEOF = true,
                  .Output = {CreateDOCTYPEToken(
                               {.Name = u8"html", .PublicIdentifier = u8"identifier", .ForceQuirks = true}),
                             CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInDOCTYPE, .Line = 1uz, .Column = 34uz}}}))

  TEST("DOCTYPEPublicIdentifierDoubleQuoted",
       "appends to public identifier when parsing any character except QuotationMark, GreaterThanSign, Null, "
       "or EOF",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPEPublicIdentifierDoubleQuoted,
                  .Input = U"<!DOCTYPE HTML PUBLIC \"identifier"}))

#pragma endregion

#pragma region DOCTYPEPublicIdentifierSingleQuoted

  TEST("DOCTYPEPublicIdentifierSingleQuoted",
       "switches to AfterDOCTYPEPublicIdentifier when parsing Apostrophe",
       (UnitTest {.ExpectedState = TokenizerState::AfterDOCTYPEPublicIdentifier,
                  .Input = U"<!DOCTYPE HTML PUBLIC 'identifier'"}))

  TEST("DOCTYPEPublicIdentifierSingleQuoted", "replaces Null with U+FFFD",
       (UnitTest {
         .ExpectedState = TokenizerState::DOCTYPEPublicIdentifierSingleQuoted,
         .Input = InsertUTF32Null(U"<!DOCTYPE HTML PUBLIC 'iden", U"tifier"),
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 28uz}}}))

  TEST("DOCTYPEPublicIdentifierSingleQuoted", "emits DOCTYPE with force-quirks when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<!DOCTYPE HTML PUBLIC 'identifier>",
         .Output = {CreateDOCTYPEToken(
           {.Name = u8"html", .PublicIdentifier = u8"identifier", .ForceQuirks = true})},
         .Errors = {{.Error = HTMLParseError::AbruptDOCTYPEPublicIdentifier, .Line = 1uz, .Column = 34uz}}}))

  TEST("DOCTYPEPublicIdentifierSingleQuoted", "emits DOCTYPE with force-quirks when EOF reached",
       (UnitTest {.Input = U"<!DOCTYPE HTML PUBLIC 'identifier",
                  .AppendEOF = true,
                  .Output = {CreateDOCTYPEToken(
                               {.Name = u8"html", .PublicIdentifier = u8"identifier", .ForceQuirks = true}),
                             CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInDOCTYPE, .Line = 1uz, .Column = 34uz}}}))

  TEST("DOCTYPEPublicIdentifierSingleQuoted",
       "appends to public identifier when parsing any character except Apostrophe, GreaterThanSign, Null, "
       "or EOF",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPEPublicIdentifierSingleQuoted,
                  .Input = U"<!DOCTYPE HTML PUBLIC 'identifier"}))

#pragma endregion

#pragma region AfterDOCTYPEPublicIdentifier

  TEST("AfterDOCTYPEPublicIdentifier",
       "ignores whitespace and switches to BetweenDOCTYPEPublicAndSystemIdentifiers when parsing whitespace",
       (UnitTest {.ExpectedState = TokenizerState::BetweenDOCTYPEPublicAndSystemIdentifiers,
                  .Input = U"<!DOCTYPE HTML PUBLIC \"id\"   \t\n\r  "}))

  TEST("AfterDOCTYPEPublicIdentifier", "switches to Data and emits DOCTYPE when parsing GreaterThanSign",
       (UnitTest {.Input = U"<!DOCTYPE HTML PUBLIC \"id\">",
                  .Output = {CreateDOCTYPEToken({.Name = u8"html", .PublicIdentifier = u8"id"})}}))

  TEST("AfterDOCTYPEPublicIdentifier",
       "switches to DOCTYPESystemIdentifierDoubleQuoted with parser error when parsing QuotationMark",
       (UnitTest {
         .ExpectedState = TokenizerState::DOCTYPESystemIdentifierDoubleQuoted,
         .Input = U"<!DOCTYPE HTML PUBLIC \"id\"\"",
         .Errors = {{.Error = HTMLParseError::MissingWhitespaceBetweenDOCTYPEPublicAndSystemIdentifiers,
                     .Line = 1uz,
                     .Column = 27uz}}}))

  TEST("AfterDOCTYPEPublicIdentifier",
       "switches to DOCTYPESystemIdentifierSingleQuoted with parser error when parsing Apostrophe",
       (UnitTest {
         .ExpectedState = TokenizerState::DOCTYPESystemIdentifierSingleQuoted,
         .Input = U"<!DOCTYPE HTML PUBLIC \"id\"'",
         .Errors = {{.Error = HTMLParseError::MissingWhitespaceBetweenDOCTYPEPublicAndSystemIdentifiers,
                     .Line = 1uz,
                     .Column = 27uz}}}))

  TEST("AfterDOCTYPEPublicIdentifier", "emits DOCTYPE with force-quirks when EOF reached",
       (UnitTest {
         .Input = U"<!DOCTYPE HTML PUBLIC \"id\"",
         .AppendEOF = true,
         .Output = {CreateDOCTYPEToken({.Name = u8"html", .PublicIdentifier = u8"id", .ForceQuirks = true}),
                    CreateEOFToken()},
         .Errors = {{.Error = HTMLParseError::EOFInDOCTYPE, .Line = 1uz, .Column = 27uz}}}))

  TEST("AfterDOCTYPEPublicIdentifier", "treats anything else as unexpected and switches to BogusDOCTYPE",
       (UnitTest {.ExpectedState = TokenizerState::BogusDOCTYPE,
                  .Input = U"<!DOCTYPE HTML PUBLIC \"id\"a",
                  .Errors = {{.Error = HTMLParseError::MissingQuoteBeforeDOCTYPESystemIdentifier,
                              .Line = 1uz,
                              .Column = 27uz}}}))

#pragma endregion

#pragma region BetweenDOCTYPEPublicAndSystemIdentifiers

  TEST("BetweenDOCTYPEPublicAndSystemIdentifiers",
       "ignores whitespace and stays in the same state when parsing whitespace",
       (UnitTest {.InitialState = TokenizerState::BetweenDOCTYPEPublicAndSystemIdentifiers,
                  .ExpectedState = TokenizerState::BetweenDOCTYPEPublicAndSystemIdentifiers,
                  .Input = U"   \t\n\r"}))

  TEST("BetweenDOCTYPEPublicAndSystemIdentifiers",
       "switches to Data and emits DOCTYPE when parsing GreaterThanSign",
       (UnitTest {.Input = U"<!DOCTYPE HTML PUBLIC \"id\">",
                  .Output = {CreateDOCTYPEToken({.Name = u8"html", .PublicIdentifier = u8"id"})}}))

  TEST("BetweenDOCTYPEPublicAndSystemIdentifiers",
       "switches to DOCTYPESystemIdentifierDoubleQuoted when parsing QuotationMark",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPESystemIdentifierDoubleQuoted,
                  .Input = U"<!DOCTYPE HTML PUBLIC \"id\" \""}))

  TEST("BetweenDOCTYPEPublicAndSystemIdentifiers",
       "switches to DOCTYPESystemIdentifierSingleQuoted when parsing Apostrophe",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPESystemIdentifierSingleQuoted,
                  .Input = U"<!DOCTYPE HTML PUBLIC \"id\" '"}))

  TEST("BetweenDOCTYPEPublicAndSystemIdentifiers", "emits DOCTYPE with force-quirks when EOF reached",
       (UnitTest {
         .Input = U"<!DOCTYPE HTML PUBLIC \"id\"",
         .AppendEOF = true,
         .Output = {CreateDOCTYPEToken({.Name = u8"html", .PublicIdentifier = u8"id", .ForceQuirks = true}),
                    CreateEOFToken()},
         .Errors = {{.Error = HTMLParseError::EOFInDOCTYPE, .Line = 1uz, .Column = 27uz}}}))

  TEST("BetweenDOCTYPEPublicAndSystemIdentifiers",
       "treats anything else as unexpected and switches to BogusDOCTYPE",
       (UnitTest {.ExpectedState = TokenizerState::BogusDOCTYPE,
                  .Input = U"<!DOCTYPE HTML PUBLIC \"id\"a",
                  .Errors = {{.Error = HTMLParseError::MissingQuoteBeforeDOCTYPESystemIdentifier,
                              .Line = 1uz,
                              .Column = 27uz}}}))

#pragma endregion

#pragma region AfterDOCTYPESystemKeyword

  TEST("AfterDOCTYPESystemKeyword",
       "ignores whitespace and switches to BeforeDOCTYPESystemIdentifier when parsing whitespace",
       (UnitTest {.ExpectedState = TokenizerState::BeforeDOCTYPESystemIdentifier,
                  .Input = U"<!DOCTYPE HTML SYSTEM   \t\n\r  "}))

  TEST("AfterDOCTYPESystemKeyword",
       "switches to DOCTYPESystemIdentifierDoubleQuoted with parser error when parsing QuotationMark",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPESystemIdentifierDoubleQuoted,
                  .Input = U"<!DOCTYPE HTML SYSTEM\"",
                  .Errors = {{.Error = HTMLParseError::MissingWhitespaceAfterDOCTYPESystemKeyword,
                              .Line = 1uz,
                              .Column = 22uz}}}))

  TEST("AfterDOCTYPESystemKeyword",
       "switches to DOCTYPESystemIdentifierSingleQuoted with parser error when parsing Apostrophe",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPESystemIdentifierSingleQuoted,
                  .Input = U"<!DOCTYPE HTML SYSTEM'",
                  .Errors = {{.Error = HTMLParseError::MissingWhitespaceAfterDOCTYPESystemKeyword,
                              .Line = 1uz,
                              .Column = 22uz}}}))

  TEST("AfterDOCTYPESystemKeyword", "emits DOCTYPE with force-quirks when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<!DOCTYPE HTML SYSTEM>",
         .Output = {CreateDOCTYPEToken({.Name = u8"html", .ForceQuirks = true})},
         .Errors = {{.Error = HTMLParseError::MissingDOCTYPESystemIdentifier, .Line = 1uz, .Column = 22uz}}}))

  TEST("AfterDOCTYPESystemKeyword", "emits DOCTYPE with force-quirks when EOF reached",
       (UnitTest {.Input = U"<!DOCTYPE HTML SYSTEM",
                  .AppendEOF = true,
                  .Output = {CreateDOCTYPEToken({.Name = u8"html", .ForceQuirks = true}), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInDOCTYPE, .Line = 1uz, .Column = 22uz}}}))

  TEST("AfterDOCTYPESystemKeyword", "treats anything else as unexpected and switches to BogusDOCTYPE",
       (UnitTest {.ExpectedState = TokenizerState::BogusDOCTYPE,
                  .Input = U"<!DOCTYPE HTML SYSTEMa",
                  .Errors = {{.Error = HTMLParseError::MissingQuoteBeforeDOCTYPESystemIdentifier,
                              .Line = 1uz,
                              .Column = 22uz}}}))

#pragma endregion

#pragma region BeforeDOCTYPESystemIdentifier

  TEST("BeforeDOCTYPESystemIdentifier",
       "ignores whitespace and stays in the same state when parsing whitespace",
       (UnitTest {.InitialState = TokenizerState::BeforeDOCTYPESystemIdentifier,
                  .ExpectedState = TokenizerState::BeforeDOCTYPESystemIdentifier,
                  .Input = U"   \t\n\r"}))

  TEST("BeforeDOCTYPESystemIdentifier",
       "switches to DOCTYPESystemIdentifierDoubleQuoted when parsing QuotationMark",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPESystemIdentifierDoubleQuoted,
                  .Input = U"<!DOCTYPE HTML SYSTEM \""}))

  TEST("BeforeDOCTYPESystemIdentifier",
       "switches to DOCTYPESystemIdentifierSingleQuoted when parsing Apostrophe",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPESystemIdentifierSingleQuoted,
                  .Input = U"<!DOCTYPE HTML SYSTEM '"}))

  TEST("BeforeDOCTYPESystemIdentifier", "emits DOCTYPE with force-quirks when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<!DOCTYPE HTML SYSTEM>",
         .Output = {CreateDOCTYPEToken({.Name = u8"html", .ForceQuirks = true})},
         .Errors = {{.Error = HTMLParseError::MissingDOCTYPESystemIdentifier, .Line = 1uz, .Column = 22uz}}}))

  TEST("BeforeDOCTYPESystemIdentifier", "emits DOCTYPE with force-quirks when EOF reached",
       (UnitTest {.Input = U"<!DOCTYPE HTML SYSTEM",
                  .AppendEOF = true,
                  .Output = {CreateDOCTYPEToken({.Name = u8"html", .ForceQuirks = true}), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInDOCTYPE, .Line = 1uz, .Column = 22uz}}}))

  TEST("BeforeDOCTYPESystemIdentifier", "treats anything else as unexpected and switches to BogusDOCTYPE",
       (UnitTest {.ExpectedState = TokenizerState::BogusDOCTYPE,
                  .Input = U"<!DOCTYPE HTML SYSTEMa",
                  .Errors = {{.Error = HTMLParseError::MissingQuoteBeforeDOCTYPESystemIdentifier,
                              .Line = 1uz,
                              .Column = 22uz}}}))

#pragma endregion

#pragma region DOCTYPESystemIdentifierDoubleQuoted

  TEST("DOCTYPESystemIdentifierDoubleQuoted",
       "switches to AfterDOCTYPESystemIdentifier when parsing QuotationMark",
       (UnitTest {.ExpectedState = TokenizerState::AfterDOCTYPESystemIdentifier,
                  .Input = U"<!DOCTYPE HTML SYSTEM \"identifier\""}))

  TEST("DOCTYPESystemIdentifierDoubleQuoted", "replaces Null with U+FFFD",
       (UnitTest {
         .ExpectedState = TokenizerState::DOCTYPESystemIdentifierDoubleQuoted,
         .Input = InsertUTF32Null(U"<!DOCTYPE HTML SYSTEM \"iden", U"tifier"),
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 28uz}}}))

  TEST("DOCTYPESystemIdentifierDoubleQuoted", "emits DOCTYPE with force-quirks when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<!DOCTYPE HTML SYSTEM \"identifier>",
         .Output = {CreateDOCTYPEToken(
           {.Name = u8"html", .SystemIdentifier = u8"identifier", .ForceQuirks = true})},
         .Errors = {{.Error = HTMLParseError::AbruptDOCTYPESystemIdentifier, .Line = 1uz, .Column = 34uz}}}))

  TEST("DOCTYPESystemIdentifierDoubleQuoted", "emits DOCTYPE with force-quirks when EOF reached",
       (UnitTest {.Input = U"<!DOCTYPE HTML SYSTEM \"identifier",
                  .AppendEOF = true,
                  .Output = {CreateDOCTYPEToken(
                               {.Name = u8"html", .SystemIdentifier = u8"identifier", .ForceQuirks = true}),
                             CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInDOCTYPE, .Line = 1uz, .Column = 34uz}}}))

  TEST("DOCTYPESystemIdentifierDoubleQuoted",
       "appends to system identifier when parsing any character except QuotationMark, GreaterThanSign, Null, "
       "or EOF",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPESystemIdentifierDoubleQuoted,
                  .Input = U"<!DOCTYPE HTML SYSTEM \"identifier"}))

#pragma endregion

#pragma region DOCTYPESystemIdentifierSingleQuoted

  TEST("DOCTYPESystemIdentifierSingleQuoted",
       "switches to AfterDOCTYPESystemIdentifier when parsing Apostrophe",
       (UnitTest {.ExpectedState = TokenizerState::AfterDOCTYPESystemIdentifier,
                  .Input = U"<!DOCTYPE HTML SYSTEM 'identifier'"}))

  TEST("DOCTYPESystemIdentifierSingleQuoted", "replaces Null with U+FFFD",
       (UnitTest {
         .ExpectedState = TokenizerState::DOCTYPESystemIdentifierSingleQuoted,
         .Input = InsertUTF32Null(U"<!DOCTYPE HTML SYSTEM 'iden", U"tifier"),
         .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 28uz}}}))

  TEST("DOCTYPESystemIdentifierSingleQuoted", "emits DOCTYPE with force-quirks when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<!DOCTYPE HTML SYSTEM 'identifier>",
         .Output = {CreateDOCTYPEToken(
           {.Name = u8"html", .SystemIdentifier = u8"identifier", .ForceQuirks = true})},
         .Errors = {{.Error = HTMLParseError::AbruptDOCTYPESystemIdentifier, .Line = 1uz, .Column = 34uz}}}))

  TEST("DOCTYPESystemIdentifierSingleQuoted", "emits DOCTYPE with force-quirks when EOF reached",
       (UnitTest {.Input = U"<!DOCTYPE HTML SYSTEM 'identifier",
                  .AppendEOF = true,
                  .Output = {CreateDOCTYPEToken(
                               {.Name = u8"html", .SystemIdentifier = u8"identifier", .ForceQuirks = true}),
                             CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInDOCTYPE, .Line = 1uz, .Column = 34uz}}}))

  TEST("DOCTYPESystemIdentifierSingleQuoted",
       "appends to system identifier when parsing any character except Apostrophe, GreaterThanSign, Null, or "
       "EOF",
       (UnitTest {.ExpectedState = TokenizerState::DOCTYPESystemIdentifierSingleQuoted,
                  .Input = U"<!DOCTYPE HTML SYSTEM 'identifiera"}))

#pragma endregion

#pragma region AfterDOCTYPESystemIdentifier

  TEST("AfterDOCTYPESystemIdentifier",
       "ignores whitespace and stays in the same state when parsing whitespace",
       (UnitTest {.InitialState = TokenizerState::AfterDOCTYPESystemIdentifier,
                  .ExpectedState = TokenizerState::AfterDOCTYPESystemIdentifier,
                  .Input = U"   \t\n\r"}))

  TEST("AfterDOCTYPESystemIdentifier", "switches to Data and emits DOCTYPE when parsing GreaterThanSign",
       (UnitTest {.Input = U"<!DOCTYPE HTML SYSTEM \"id\">",
                  .Output = {CreateDOCTYPEToken({.Name = u8"html", .SystemIdentifier = u8"id"})}}))

  TEST("AfterDOCTYPESystemIdentifier", "emits DOCTYPE with force-quirks when EOF reached",
       (UnitTest {
         .Input = U"<!DOCTYPE HTML SYSTEM \"id\"",
         .AppendEOF = true,
         .Output = {CreateDOCTYPEToken({.Name = u8"html", .SystemIdentifier = u8"id", .ForceQuirks = true}),
                    CreateEOFToken()},
         .Errors = {{.Error = HTMLParseError::EOFInDOCTYPE, .Line = 1uz, .Column = 27uz}}}))

  TEST("AfterDOCTYPESystemIdentifier", "treats anything else as unexpected and switches to BogusDOCTYPE",
       (UnitTest {.ExpectedState = TokenizerState::BogusDOCTYPE,
                  .Input = U"<!DOCTYPE HTML SYSTEM \"id\"a",
                  .Errors = {{.Error = HTMLParseError::UnexpectedCharacterAfterDOCTYPESystemIdentifier,
                              .Line = 1uz,
                              .Column = 27uz}}}))

#pragma endregion

#pragma region BogusDOCTYPE

  TEST("BogusDOCTYPE", "emits DOCTYPE when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<!DOCTYPE HTML PUBLIC \"id\" a random text >",
         .Output = {CreateDOCTYPEToken({.Name = u8"html", .PublicIdentifier = u8"id", .ForceQuirks = true})},
         .Errors = {{.Error = HTMLParseError::MissingQuoteBeforeDOCTYPESystemIdentifier,
                     .Line = 1uz,
                     .Column = 28uz}}}))

  TEST("BogusDOCTYPE", "emits DOCTYPE when EOF reached",
       (UnitTest {
         .Input = U"<!DOCTYPE HTML PUBLIC \"id\" a random text",
         .AppendEOF = true,
         .Output = {CreateDOCTYPEToken({.Name = u8"html", .PublicIdentifier = u8"id", .ForceQuirks = true})},
         .Errors = {{.Error = HTMLParseError::MissingQuoteBeforeDOCTYPESystemIdentifier,
                     .Line = 1uz,
                     .Column = 28uz}}}))

  TEST("BogusDOCTYPE", "ignores Null with parse error",
       (UnitTest {
         .ExpectedState = TokenizerState::BogusDOCTYPE,
         .Input = InsertUTF32Null(U"<!DOCTYPE HTML PUBLIC \"id\" a random text"),
         .Errors = {
           {.Error = HTMLParseError::MissingQuoteBeforeDOCTYPESystemIdentifier, .Line = 1uz, .Column = 28uz},
           {.Error = HTMLParseError::UnexpectedNullCharacter, .Line = 1uz, .Column = 41uz}}}))

  TEST("BogusDOCTYPE", "ignores characters until GreaterThanSign or EOF",
       (UnitTest {
         .Input = U"<!DOCTYPE HTML PUBLIC \"id\" a random text >",
         .Output = {CreateDOCTYPEToken({.Name = u8"html", .PublicIdentifier = u8"id", .ForceQuirks = true})},
         .Errors = {{.Error = HTMLParseError::MissingQuoteBeforeDOCTYPESystemIdentifier,
                     .Line = 1uz,
                     .Column = 28uz}}}))

#pragma endregion

#pragma region CDATASection

  TEST("CDATASection", "switches to CDATASectionEnd when parsing RightSquareBracket",
       (UnitTest {.ExpectedState = TokenizerState::CDATASectionEnd,
                  .Input = U"<![CDATA[]]",
                  .CDATASectionAllowed = true}))

  TEST("CDATASection", "emits EndOfFile with parser error when EOF reached",
       (UnitTest {.Input = U"<![CDATA[A",
                  .AppendEOF = true,
                  .CDATASectionAllowed = true,
                  .Output = {CreateCharacterToken(u8"A"), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInCDATA, .Line = 1uz, .Column = 11uz}}}))

  TEST("CDATASection", "emits Character tokens for all characters except RightSquareBracket and EOF",
       (UnitTest {.ExpectedState = TokenizerState::CDATASection,
                  .Input = U"<![CDATA[ABC",
                  .CDATASectionAllowed = true,
                  .Output = {CreateCharacterToken(u8"ABC")}}))

#pragma endregion

#pragma region CDATASectionBracket

  TEST("CDATASectionBracket", "switches to CDATASectionEnd when parsing RightSquareBracket",
       (UnitTest {
         .ExpectedState = TokenizerState::CDATASectionEnd,
         .Input = U"<![CDATA[]]",
         .CDATASectionAllowed = true,
       }))

  TEST("CDATASectionBracket",
       "emits RightSquareBracket and switches back to CDATASection when parsing any character except "
       "RightSquareBracket or EOF",
       (UnitTest {.ExpectedState = TokenizerState::CDATASection,
                  .Input = U"<![CDATA[]A",
                  .CDATASectionAllowed = true,
                  .Output = {CreateCharacterToken(u8"]A")}}))

  TEST("CDATASectionBracket",
       "emits Character token for RightSquareBracket and EndOfFile with parser error when EOF reached",
       (UnitTest {.Input = U"<![CDATA[]",
                  .AppendEOF = true,
                  .CDATASectionAllowed = true,
                  .Output = {CreateCharacterToken(u8"]"), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInCDATA, .Line = 1uz, .Column = 11uz}}}))

#pragma endregion

#pragma region CDATASectionEnd

  TEST("CDATASectionEnd", "emits RightSquareBracket when parsing RightSquareBracket",
       (UnitTest {.ExpectedState = TokenizerState::CDATASectionEnd,
                  .Input = U"<![CDATA[]]]",
                  .CDATASectionAllowed = true,
                  .Output = {CreateCharacterToken(u8"]")}}))

  TEST("CDATASectionEnd",
       "emits Character tokens for two RightSquareBrackets and EndOfFile with parser error when EOF reached",
       (UnitTest {.Input = U"<![CDATA[]]]",
                  .AppendEOF = true,
                  .CDATASectionAllowed = true,
                  .Output = {CreateCharacterToken(u8"]]]"), CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInCDATA, .Line = 1uz, .Column = 13uz}}}))

  TEST("CDATASectionEnd", "switches to Data when parsing GreaterThanSign",
       (UnitTest {.Input = U"<![CDATA[]]>", .CDATASectionAllowed = true}))

  TEST("CDATASectionEnd",
       "emits Character tokens for two RightSquareBrackets and switches back to CDATASection when parsing "
       "any character except RightSquareBracket or GreaterThanSign",
       (UnitTest {.ExpectedState = TokenizerState::CDATASection,
                  .Input = U"<![CDATA[]]A",
                  .CDATASectionAllowed = true,
                  .Output = {CreateCharacterToken(u8"]]A")}}))

#pragma endregion
}