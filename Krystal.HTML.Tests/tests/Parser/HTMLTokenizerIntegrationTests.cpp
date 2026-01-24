#include "Krystal.HTML/Parser/HTMLTokenizer.hpp"
#include <catch_all.hpp>

namespace
{
  using namespace Krys;
  using namespace Krys::HTML;

  bool CompareDataBuffers(const HTMLToken::DataBuffer &a, const HTMLToken::DataBuffer &b) noexcept
  {
    bool result = true;
    if (a.size() != b.size())
    {
      result = false;
    }
    if (result)
    {
      for (size_t i = 0uz; i < a.size(); ++i)
      {
        if (a[i] != b[i])
        {
          result = false;
          break;
        }
      }
    }

    return result;
  }

  void AddToTokenName(HTMLToken &token, utf32_string name)
  {
    for (char32 c : name)
    {
      token.AppendToName(c);
    }
  }

  void AddToTokenCharacters(HTMLToken &token, utf32_string characters)
  {
    for (char32 c : characters)
    {
      token.AppendToCharacters(c);
    }
  }

  void SetTokenAttribute(HTMLToken &token, utf32_string name, utf32_string value)
  {
    token.BeginAttribute();
    for (char32 c : name)
    {
      token.AppendToCurrentAttributeName(c);
    }
    for (char32 c : value)
    {
      token.AppendToCurrentAttributeValue(c);
    }
    token.EndAttribute();
  }
}

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("HTMLTokenizer(IntegrationTest1)", "[HTML][Tokenizer]")
  {
#pragma region ExpectedTokens

    List<HTMLToken> expectedTokens = {};
    // DOCTYPE
    expectedTokens.emplace_back().BeginDOCTYPE();
    AddToTokenName(expectedTokens.back(), U"html");

    // Characters
    expectedTokens.emplace_back();
    AddToTokenCharacters(expectedTokens.back(), U"\n        ");

    // HTML Start Tag
    expectedTokens.emplace_back().BeginStartTag(U'h');
    AddToTokenName(expectedTokens.back(), U"tml");
    SetTokenAttribute(expectedTokens.back(), U"lang", U"en");

    // Characters
    expectedTokens.emplace_back();
    AddToTokenCharacters(expectedTokens.back(), U"\n        ");

    // HEAD Start Tag
    expectedTokens.emplace_back().BeginStartTag(U'h');
    AddToTokenName(expectedTokens.back(), U"ead");

    // Characters
    expectedTokens.emplace_back();
    AddToTokenCharacters(expectedTokens.back(), U"\n            ");

    // META Start Tag
    expectedTokens.emplace_back().BeginStartTag(U'm');
    AddToTokenName(expectedTokens.back(), U"eta");
    SetTokenAttribute(expectedTokens.back(), U"charset", U"UTF-8");

    // Characters
    expectedTokens.emplace_back();
    AddToTokenCharacters(expectedTokens.back(), U"\n            ");

    // META Start Tag
    expectedTokens.emplace_back().BeginStartTag(U'm');
    AddToTokenName(expectedTokens.back(), U"eta");
    SetTokenAttribute(expectedTokens.back(), U"name", U"viewport");
    SetTokenAttribute(expectedTokens.back(), U"content", U"width=device-width, initial-scale=1.0");

    // Characters
    expectedTokens.emplace_back();
    AddToTokenCharacters(expectedTokens.back(), U"\n            ");

    // LINK Start Tag
    expectedTokens.emplace_back().BeginStartTag(U'l');
    AddToTokenName(expectedTokens.back(), U"ink");
    SetTokenAttribute(expectedTokens.back(), U"type", U"text/css");
    SetTokenAttribute(expectedTokens.back(), U"href", U"index.css");

    // Characters
    expectedTokens.emplace_back();
    AddToTokenCharacters(expectedTokens.back(), U"\n            ");

    // TITLE Start Tag
    expectedTokens.emplace_back().BeginStartTag(U't');
    AddToTokenName(expectedTokens.back(), U"itle");

    // Characters
    expectedTokens.emplace_back();
    AddToTokenCharacters(expectedTokens.back(), U"Document");

    // TITLE End Tag
    expectedTokens.emplace_back().BeginEndTag(U't');
    AddToTokenName(expectedTokens.back(), U"itle");

    // Characters
    expectedTokens.emplace_back();
    AddToTokenCharacters(expectedTokens.back(), U"\n        ");

    // HEAD End Tag
    expectedTokens.emplace_back().BeginEndTag(U'h');
    AddToTokenName(expectedTokens.back(), U"ead");

    // Characters
    expectedTokens.emplace_back();
    AddToTokenCharacters(expectedTokens.back(), U"\n        ");

    // BODY Start Tag
    expectedTokens.emplace_back().BeginStartTag(U'b');
    AddToTokenName(expectedTokens.back(), U"ody");
    SetTokenAttribute(expectedTokens.back(), U"style", U"background-color: maroon;");

    // Characters
    expectedTokens.emplace_back();
    AddToTokenCharacters(expectedTokens.back(), U"\n            ");

    // DIV Start Tag
    expectedTokens.emplace_back().BeginStartTag(U'd');
    AddToTokenName(expectedTokens.back(), U"iv");
    SetTokenAttribute(expectedTokens.back(), U"style", U"\n              box-sizing: border-box;");

    // Characters
    expectedTokens.emplace_back();
    AddToTokenCharacters(expectedTokens.back(), U"Hello, Krystal UI");

    // DIV End Tag
    expectedTokens.emplace_back().BeginEndTag(U'd');
    AddToTokenName(expectedTokens.back(), U"iv");

    // Characters
    expectedTokens.emplace_back();
    AddToTokenCharacters(expectedTokens.back(), U"\n            ");

    // SCRIPT Start Tag
    expectedTokens.emplace_back().BeginStartTag(U's');
    AddToTokenName(expectedTokens.back(), U"cript");
    SetTokenAttribute(expectedTokens.back(), U"type", U"javascript");
    SetTokenAttribute(expectedTokens.back(), U"src", U"index.js");

    // SCRIPT End Tag
    expectedTokens.emplace_back().BeginEndTag(U's');
    AddToTokenName(expectedTokens.back(), U"cript");

    // Characters
    expectedTokens.emplace_back();
    AddToTokenCharacters(expectedTokens.back(), U"\n        ");

    // BODY End Tag
    expectedTokens.emplace_back().BeginEndTag(U'b');
    AddToTokenName(expectedTokens.back(), U"ody");

    // Characters
    expectedTokens.emplace_back();
    AddToTokenCharacters(expectedTokens.back(), U"\n        ");

    // HTML End Tag
    expectedTokens.emplace_back().BeginEndTag(U'h');
    AddToTokenName(expectedTokens.back(), U"tml");

    // EOF
    expectedTokens.emplace_back().SetAsEOF();

#pragma endregion

    utf32_string input = UR"html(<!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <link type="text/css" href="index.css">
            <title>Document</title>
        </head>
        <body style="background-color: maroon;">
            <div style="
              box-sizing: border-box;">Hello, Krystal UI</div>
            <script type="javascript" src="index.js"></script>
        </body
>
        </html>)html";

    HTMLInputStream inputStream;
    inputStream.Append(std::move(input), IsEOF(true));

    HTMLTokenizer tokenizer(inputStream);
    const auto &errors = tokenizer.GetParseErrors();
    size_t expectedErrorCount = 0uz;

    size_t tokenIndex = 0uz;
    while (true)
    {
      const HTMLToken &expectedToken = expectedTokens[tokenIndex++];
      NextTokenPtr tokenOpt = tokenizer.NextToken();
      REQUIRE(tokenOpt);

      const HTMLToken &token = *tokenOpt;

      REQUIRE(token.GetType() == expectedToken.GetType());
      if (token.GetType() == HTMLToken::Type::EndOfFile)
      {
        break;
      }

      REQUIRE(CompareDataBuffers(token.GetDataBuffer(), expectedToken.GetDataBuffer()));

      if (token.GetType() == HTMLToken::Type::StartTag || token.GetType() == HTMLToken::Type::EndTag)
      {
        const auto &expectedAttributes = expectedToken.GetAttributes();
        const auto &attributes = token.GetAttributes();

        REQUIRE(attributes.size() == expectedAttributes.size());
        for (size_t i = 0uz; i < attributes.size(); ++i)
        {
          REQUIRE(CompareDataBuffers(attributes[i].Name, expectedAttributes[i].Name));
          REQUIRE(CompareDataBuffers(attributes[i].Value, expectedAttributes[i].Value));
        }
      }
    }
    REQUIRE(errors.size() == expectedErrorCount);
  }
}