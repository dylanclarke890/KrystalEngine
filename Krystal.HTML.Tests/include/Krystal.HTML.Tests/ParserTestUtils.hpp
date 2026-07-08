#pragma once

#include "Krystal.HTML/HTML/Parser/HTMLInputStream.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLToken.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenizer.hpp"
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
    utf8_string Name;
    utf8_string Value;
  };

  struct ExpectedDOCTYPE
  {
    utf8_string Name;
    utf8_string PublicIdentifier;
    utf8_string SystemIdentifier;
    bool ForceQuirks {false};
  };

  struct ExpectedCharacter
  {
    utf8_string Data;
  };

  struct ExpectedTag
  {
    utf8_string Name;
    List<Attr> Attributes;
    bool SelfClosing {false};
  };

  struct ExpectedComment
  {
    utf8_string Data;
  };

  struct ExpectedEOF
  {
  };

  struct ExpectedToken
  {
    using TokenVariant =
      Variant<ExpectedDOCTYPE, ExpectedCharacter, ExpectedTag, ExpectedComment, ExpectedEOF>;

    HTML::HTMLTokenType Type;
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
    return ExpectedToken {.Type = HTML::HTMLTokenType::DOCTYPE, .Token = expected};
  }

  KRYS_NODISCARD inline ExpectedToken CreateCharacterToken(const utf8_string &characters) noexcept
  {
    return ExpectedToken {.Type = HTML::HTMLTokenType::Character, .Token = ExpectedCharacter {characters}};
  }

  KRYS_NODISCARD inline ExpectedToken CreateCommentToken(const utf8_string &comment) noexcept
  {
    return ExpectedToken {.Type = HTML::HTMLTokenType::Comment, .Token = ExpectedComment {comment}};
  }

  KRYS_NODISCARD inline ExpectedToken CreateStartTagToken(const ExpectedTag &expected) noexcept
  {
    return ExpectedToken {.Type = HTML::HTMLTokenType::StartTag, .Token = expected};
  }

  KRYS_NODISCARD inline ExpectedToken CreateEndTagToken(const ExpectedTag &expected) noexcept
  {
    return ExpectedToken {.Type = HTML::HTMLTokenType::EndTag, .Token = expected};
  }

  KRYS_NODISCARD inline ExpectedToken CreateEOFToken() noexcept
  {
    return ExpectedToken {.Type = HTML::HTMLTokenType::EndOfFile, .Token = ExpectedEOF {}};
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

  KRYS_NODISCARD inline utf32_string &&InsertUTF32Null(utf32_string &&str,
                                                       utf32_string &&suffix = U"") noexcept
  {
    str.push_back(U'\0');
    str.append(std::move(suffix));
    return std::move(str);
  }

  KRYS_NODISCARD inline utf8_string &&InsertUTF8Null(utf8_string &&str, utf8_string &&suffix = u8"") noexcept
  {
    str.push_back(u8'\0');
    str.append(std::move(suffix));
    return std::move(str);
  }

  inline void CheckDOCTYPE(HTML::HTMLToken &token, const ExpectedDOCTYPE &expected) noexcept
  {
    CHECK(Compare(token.Data(), expected.Name));

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
    if (!Compare(token.Data(), expected.Data))
    {
      INFO("Expected:\n"
           + string(reinterpret_cast<const char *>(expected.Data.data()), expected.Data.size()));
      INFO("Actual:\n" + string(reinterpret_cast<const char *>(token.Data().data()), token.Data().size()));
      CHECK(("CheckCharacter" && false));
    }
  }

  inline void CheckTag(HTML::HTMLToken &token, const ExpectedTag &expected) noexcept
  {
    CHECK(Compare(token.Data(), expected.Name));

    if (token.Type() == HTML::HTMLTokenType::StartTag)
    {
      CHECK(token.IsSelfClosing() == expected.SelfClosing);
    }

    CHECK(token.Attributes().size() == expected.Attributes.size());
    for (size_t i = 0uz; i < expected.Attributes.size(); ++i)
    {
      CHECK(Compare(token.Attributes()[i].Name, expected.Attributes[i].Name));
      CHECK(Compare(token.Attributes()[i].Value, expected.Attributes[i].Value));
    }
  }

  inline void CheckComment(HTML::HTMLToken &token, const ExpectedComment &expected) noexcept
  {
    CHECK(Compare(token.Data(), expected.Data));
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
    const auto &errors = tokenizer.ParseErrors();

    size_t tokenIndex = 0uz;
    while (true)
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(token);

      const auto &expected = testCase.Output[tokenIndex++];
      REQUIRE(token->Type() == expected.Type);

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
      CHECK(errors[i].Error == testCase.Errors[i].Error);
      CHECK(errors[i].Location.Line == testCase.Errors[i].Line);
      CHECK(errors[i].Location.Column == testCase.Errors[i].Column);
    }
  }

  inline void DoUnitTest(UnitTest &&testCase) noexcept
  {
    using namespace HTML;

    HTMLInputStream inputStream;
    inputStream.Append(std::move(testCase.Input), IsEOF(testCase.AppendEOF));

    HTMLTokenizer tokenizer(inputStream);
    tokenizer.State(testCase.InitialState);

    if (testCase.Setup)
    {
      testCase.Setup(tokenizer);
    }

    const auto &errors = tokenizer.ParseErrors();

    if (!testCase.Output.size())
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(errors.size() == testCase.Errors.size());

      for (size_t i = 0uz; i < errors.size(); ++i)
      {
        CHECK(errors[i].Error == testCase.Errors[i].Error);
        CHECK(errors[i].Location.Line == testCase.Errors[i].Line);
        CHECK(errors[i].Location.Column == testCase.Errors[i].Column);
      }

      CHECK(tokenizer.State() == testCase.ExpectedState);
      return;
    }

    REQUIRE(tokenizer.State() == testCase.InitialState);
    for (const auto &expected : testCase.Output)
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(token);
      REQUIRE(token->Type() == expected.Type);

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

    CHECK(tokenizer.State() == testCase.ExpectedState);

    REQUIRE(errors.size() == testCase.Errors.size());
    for (size_t i = 0uz; i < errors.size(); ++i)
    {
      CHECK(errors[i].Error == testCase.Errors[i].Error);
      CHECK(errors[i].Location.Line == testCase.Errors[i].Line);
      CHECK(errors[i].Location.Column == testCase.Errors[i].Column);
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