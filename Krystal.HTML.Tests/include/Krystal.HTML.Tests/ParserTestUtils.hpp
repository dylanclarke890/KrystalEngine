#pragma once

#include "Krystal.HTML/Parser/HTMLInputStream.hpp"
#include "Krystal.HTML/Parser/HTMLParseError.hpp"
#include "Krystal.HTML/Parser/HTMLToken.hpp"
#include "Krystal.HTML/Parser/HTMLTokenizer.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Variant.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  enum class TagType
  {
    Start,
    End
  };

  struct Attr
  {
    utf32_string Name;
    utf32_string Value;
  };

  struct ExpectedDOCTYPE
  {
    utf32_string Name;
    utf32_string PublicIdentifier;
    utf32_string SystemIdentifier;
    bool ForceQuirks {false};
  };

  struct ExpectedCharacter
  {
    utf32_string Data;
  };

  struct ExpectedTag
  {
    utf32_string Name;
    List<Attr> Attributes;
    bool SelfClosing {false};
  };

  struct ExpectedComment
  {
    utf32_string Data;
  };

  struct ExpectedEOF
  {
  };

  struct ExpectedToken
  {
    using TokenVariant =
      Variant<ExpectedDOCTYPE, ExpectedCharacter, ExpectedTag, ExpectedComment, ExpectedEOF>;

    HTML::HTMLToken::Type Type;
    TokenVariant Token;
  };

  struct ExpectedError
  {
    HTML::HTMLParseError Error;
    size_t Line {0uz};
    size_t Column {0uz};
  };

  struct HTMLIntegrationTestCase
  {
    utf32_string Input;
    List<ExpectedToken> Output;
    List<ExpectedError> Errors;
  };

  KRYS_NODISCARD inline ExpectedToken CreateDOCTYPEToken(const ExpectedDOCTYPE &expected) noexcept
  {
    return ExpectedToken {.Type = HTML::HTMLToken::Type::DOCTYPE, .Token = expected};
  }

  KRYS_NODISCARD inline ExpectedToken CreateCharacterToken(const utf32_string &characters) noexcept
  {
    return ExpectedToken {.Type = HTML::HTMLToken::Type::Character, .Token = ExpectedCharacter {characters}};
  }

  KRYS_NODISCARD inline ExpectedToken CreateCommentToken(const utf32_string &comment) noexcept
  {
    return ExpectedToken {.Type = HTML::HTMLToken::Type::EndOfFile, .Token = ExpectedComment {comment}};
  }

  KRYS_NODISCARD inline ExpectedToken CreateStartTagToken(const ExpectedTag &expected) noexcept
  {
    return ExpectedToken {.Type = HTML::HTMLToken::Type::StartTag, .Token = expected};
  }

  KRYS_NODISCARD inline ExpectedToken CreateEndTagToken(const ExpectedTag &expected) noexcept
  {
    return ExpectedToken {.Type = HTML::HTMLToken::Type::EndTag, .Token = expected};
  }

  KRYS_NODISCARD inline ExpectedToken CreateEOFToken() noexcept
  {
    return ExpectedToken {.Type = HTML::HTMLToken::Type::EndOfFile, .Token = ExpectedEOF {}};
  }

  KRYS_NODISCARD inline bool Compare(const auto &a, const auto &b) noexcept
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

  inline void DoHTMLTest(HTMLIntegrationTestCase &&testCase) noexcept
  {
    using namespace HTML;

    HTMLInputStream inputStream;
    inputStream.Append(std::move(testCase.Input), IsEOF(true));

    HTMLTokenizer tokenizer(inputStream);
    const auto &errors = tokenizer.GetParseErrors();

    size_t tokenIndex = 0uz;
    while (true)
    {
      NextTokenPtr nextToken = tokenizer.NextToken();
      REQUIRE(nextToken);
      HTMLToken &token = *nextToken;

      const auto &expectedToken = testCase.Output[tokenIndex++];
      REQUIRE(token.GetType() == expectedToken.Type);

      if (const auto *expectedDoctypeToken = std::get_if<ExpectedDOCTYPE>(&expectedToken.Token))
      {
        REQUIRE(Compare(token.GetDataBuffer(), expectedDoctypeToken->Name));

        UniquePtr<HTML::DoctypeData> doctypeData = std::move(token.ReleaseDOCTYPEData());
        if (expectedDoctypeToken->PublicIdentifier.empty())
        {
          REQUIRE(doctypeData->PublicIdentifier.empty());
        }
        else
        {
          REQUIRE(Compare(doctypeData->PublicIdentifier, expectedDoctypeToken->PublicIdentifier));
        }

        if (expectedDoctypeToken->SystemIdentifier.empty())
        {
          REQUIRE(doctypeData->SystemIdentifier.empty());
        }
        else
        {
          REQUIRE(Compare(doctypeData->SystemIdentifier, expectedDoctypeToken->SystemIdentifier));
        }

        REQUIRE(doctypeData->ForceQuirks == expectedDoctypeToken->ForceQuirks);

        continue;
      }

      if (const auto *characterToken = std::get_if<ExpectedCharacter>(&expectedToken.Token))
      {
        REQUIRE(Compare(token.GetDataBuffer(), characterToken->Data));
        continue;
      }

      if (const auto *tagToken = std::get_if<ExpectedTag>(&expectedToken.Token))
      {
        REQUIRE(Compare(token.GetDataBuffer(), tagToken->Name));
        if (expectedToken.Type == HTML::HTMLToken::Type::StartTag)
        {
          REQUIRE(token.IsSelfClosing() == tagToken->SelfClosing);
        }
        REQUIRE(token.GetAttributes().size() == tagToken->Attributes.size());
        for (size_t i = 0uz; i < tagToken->Attributes.size(); ++i)
        {
          REQUIRE(Compare(token.GetAttributes()[i].Name, tagToken->Attributes[i].Name));
          REQUIRE(Compare(token.GetAttributes()[i].Value, tagToken->Attributes[i].Value));
        }

        continue;
      }

      if (const auto *commentToken = std::get_if<ExpectedComment>(&expectedToken.Token))
      {
        REQUIRE(Compare(token.GetDataBuffer(), commentToken->Data));
        continue;
      }

      if (std::holds_alternative<ExpectedEOF>(expectedToken.Token))
      {
        REQUIRE(errors.size() == testCase.Errors.size());
        for (size_t i = 0uz; i < errors.size(); ++i)
        {
          REQUIRE(errors[i] == testCase.Errors[i].Error);
        }

        break;
      }

      FAIL("Unknown expected token type");
    }
  }

#define HTML_INTEGRATION_TEST(name, testCase)                                                                \
  TEST_CASE("HTMLTokenizer(" name ")", "[HTML][Tokenizer][Integration]")                                                          \
  {                                                                                                          \
    DoHTMLTest(testCase);                                                                                    \
  }
}