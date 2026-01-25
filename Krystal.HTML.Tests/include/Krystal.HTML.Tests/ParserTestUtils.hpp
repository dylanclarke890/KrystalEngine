#pragma once

#include "Krystal.HTML/Parser/HTMLInputStream.hpp"
#include "Krystal.HTML/Parser/HTMLParseError.hpp"
#include "Krystal.HTML/Parser/HTMLToken.hpp"
#include "Krystal.HTML/Parser/HTMLTokenizer.hpp"
#include "Krystal.HTML/Parser/TokenizerState.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Variant.hpp"
#include <catch_all.hpp>
#include <functional>

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

  struct IntegrationTest
  {
    utf32_string Input;
    List<ExpectedToken> Output;
    List<ExpectedError> Errors;
  };

  struct UnitTest
  {
    HTML::TokenizerState InitialState {HTML::TokenizerState::Data};
    HTML::TokenizerState ExpectedState {HTML::TokenizerState::Data};
    utf32_string Input;
    bool AppendEOF {false};
    std::function<void(HTML::HTMLTokenizer &tokenizer)> Setup = nullptr;
    List<ExpectedToken> Output {};
    List<ExpectedError> Errors {};
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
    return ExpectedToken {.Type = HTML::HTMLToken::Type::Comment, .Token = ExpectedComment {comment}};
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

  KRYS_NODISCARD inline utf32_string &&InsertNull(utf32_string &&str, utf32_string &&suffix = U"") noexcept
  {
    str.push_back(U'\0');
    str.append(std::move(suffix));
    return std::move(str);
  }

  inline void CheckDOCTYPE(HTML::HTMLToken &token, const ExpectedDOCTYPE &expected) noexcept
  {
    CHECK(Compare(token.GetDataBuffer(), expected.Name));

    UniquePtr<HTML::DoctypeData> doctypeData = std::move(token.ReleaseDOCTYPEData());
    CHECK(doctypeData->ForceQuirks == expected.ForceQuirks);

    if (expected.PublicIdentifier.empty())
    {
      CHECK(doctypeData->PublicIdentifier.empty());
    }
    else
    {
      CHECK(Compare(doctypeData->PublicIdentifier, expected.PublicIdentifier));
    }

    if (expected.SystemIdentifier.empty())
    {
      CHECK(doctypeData->SystemIdentifier.empty());
    }
    else
    {
      CHECK(Compare(doctypeData->SystemIdentifier, expected.SystemIdentifier));
    }
  }

  inline void CheckCharacter(HTML::HTMLToken &token, const ExpectedCharacter &expected) noexcept
  {
    CHECK(Compare(token.GetDataBuffer(), expected.Data));
  }

  inline void CheckTag(HTML::HTMLToken &token, const ExpectedTag &expected) noexcept
  {
    CHECK(Compare(token.GetDataBuffer(), expected.Name));

    if (token.GetType() == HTML::HTMLToken::Type::StartTag)
    {
      CHECK(token.IsSelfClosing() == expected.SelfClosing);
    }

    CHECK(token.GetAttributes().size() == expected.Attributes.size());
    for (size_t i = 0uz; i < expected.Attributes.size(); ++i)
    {
      CHECK(Compare(token.GetAttributes()[i].Name, expected.Attributes[i].Name));
      CHECK(Compare(token.GetAttributes()[i].Value, expected.Attributes[i].Value));
    }
  }

  inline void CheckComment(HTML::HTMLToken &token, const ExpectedComment &expected) noexcept
  {
    CHECK(Compare(token.GetDataBuffer(), expected.Data));
  }

  inline void CheckEOF(HTML::HTMLToken &, const ExpectedEOF &) noexcept
  {
    // Nothing to check for EOF token.
  }

  inline void DoIntegrationTest(IntegrationTest &&testCase) noexcept
  {
    using namespace HTML;

    HTMLInputStream inputStream;
    inputStream.Append(std::move(testCase.Input), IsEOF(true));

    HTMLTokenizer tokenizer(inputStream);
    const auto &errors = tokenizer.GetParseErrors();

    size_t tokenIndex = 0uz;
    while (true)
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(token);

      const auto &expected = testCase.Output[tokenIndex++];
      REQUIRE(token->GetType() == expected.Type);

      if (const auto *expectedDoctypeToken = std::get_if<ExpectedDOCTYPE>(&expected.Token))
      {
        CheckDOCTYPE(*token, *expectedDoctypeToken);
        continue;
      }

      if (const auto *characterToken = std::get_if<ExpectedCharacter>(&expected.Token))
      {
        CheckCharacter(*token, *characterToken);
        continue;
      }

      if (const auto *tagToken = std::get_if<ExpectedTag>(&expected.Token))
      {
        CheckTag(*token, *tagToken);
        continue;
      }

      if (const auto *commentToken = std::get_if<ExpectedComment>(&expected.Token))
      {
        CheckComment(*token, *commentToken);
        continue;
      }

      if (const auto *eofToken = std::get_if<ExpectedEOF>(&expected.Token))
      {
        CheckEOF(*token, *eofToken);
        break;
      }

      FAIL("Unknown expected token type");
    }

    REQUIRE(errors.size() == testCase.Errors.size());
    for (size_t i = 0uz; i < errors.size(); ++i)
    {
      REQUIRE(errors[i] == testCase.Errors[i].Error);
    }
  }

  inline void DoUnitTest(UnitTest &&testCase) noexcept
  {
    using namespace HTML;

    HTMLInputStream inputStream;
    inputStream.Append(std::move(testCase.Input), IsEOF(testCase.AppendEOF));

    HTMLTokenizer tokenizer(inputStream);
    tokenizer.SetState(testCase.InitialState);

    if (testCase.Setup)
    {
      testCase.Setup(tokenizer);
    }

    const auto &errors = tokenizer.GetParseErrors();

    if (!testCase.Output.size())
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(errors.size() == testCase.Errors.size());
      
      for (size_t i = 0uz; i < errors.size(); ++i)
      {
        CHECK(errors[i] == testCase.Errors[i].Error);
      }

      CHECK(tokenizer.GetState() == testCase.ExpectedState);
      return;
    }

    REQUIRE(tokenizer.GetState() == testCase.InitialState);
    for (const auto &expected : testCase.Output)
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(token);
      REQUIRE(token->GetType() == expected.Type);

      if (const auto *expectedDoctypeToken = std::get_if<ExpectedDOCTYPE>(&expected.Token))
      {
        CheckDOCTYPE(*token, *expectedDoctypeToken);
        continue;
      }

      if (const auto *characterToken = std::get_if<ExpectedCharacter>(&expected.Token))
      {
        CheckCharacter(*token, *characterToken);
        continue;
      }

      if (const auto *tagToken = std::get_if<ExpectedTag>(&expected.Token))
      {
        CheckTag(*token, *tagToken);
        continue;
      }

      if (const auto *commentToken = std::get_if<ExpectedComment>(&expected.Token))
      {
        CheckComment(*token, *commentToken);
        continue;
      }

      if (const auto *eofToken = std::get_if<ExpectedEOF>(&expected.Token))
      {
        CheckEOF(*token, *eofToken);
        continue;
      }

      FAIL("Unknown expected token type");
    }
  }

#define INTEGRATION_TEST(name, integrationTestCase)                                                          \
  TEST_CASE("HTMLTokenizer(" name ")", "[HTML][Tokenizer][Integration]")                                     \
  {                                                                                                          \
    DoIntegrationTest(integrationTestCase);                                                                  \
  }

#define TEST(state, name, test)                                                                              \
  TEST_CASE("HTMLTokenizer(" state ") - " name, "[HTML][Tokenizer]")                                         \
  {                                                                                                          \
    DoUnitTest(test);                                                                                        \
  }
}