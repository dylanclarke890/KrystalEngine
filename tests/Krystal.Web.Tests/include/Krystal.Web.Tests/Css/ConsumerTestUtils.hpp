#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web/Css/Parser/InputStream.hpp"
#include "Krystal.Web/Css/Parser/ParserContext.hpp"
#include "Krystal.Web/Css/Parser/Tokenizer.hpp"
#include "Krystal.Web/Css/Properties/PropertyParserState.hpp"
#include <string>

namespace krys::web::css::tests
{
  /// @brief Tokenizes `css` and keeps the input alive for the token range.
  struct Tokens
  {
    InputStream Input;
    Tokenizer Tokenizer;

    explicit Tokens(utf8_stringview css) : Input(CSSOMString(css)), Tokenizer(Input)
    {
      REQUIRE(Tokenizer.PumpTokenizer());
    }
  };

  /// @brief Runs a value consumer over `css` and serialises the result.
  /// @param consumer Called as `consumer(TokenRange &, PropertyParserState &)`, returning a `RefPtr` to a
  /// value.
  /// @param context The parser context; settings the consumer checks are read from here.
  /// @param property The property being parsed, for consumers that depend on it.
  /// @return The serialised value, "<null>" if the consumer failed, or "<partial>" if it succeeded without
  /// consuming the whole input.
  template <typename F>
  KRYS_NODISCARD std::string ConsumeToText(utf8_stringview css, F consumer, const ParserContext &context,
                                           PropertyId property = PropertyId::Invalid)
  {
    PropertyParserState state {.Context = context, .CurrentProperty = property};
    Tokens tokens {css};
    auto range = tokens.Tokenizer.Tokens();
    auto value = consumer(range, state);
    if (!value)
    {
      return "<null>";
    }

    if (!range.IsAtEnd())
    {
      return "<partial>";
    }

    return CssText(*value);
  }

  /// @brief As above, with a standard-mode context.
  template <typename F>
  KRYS_NODISCARD std::string ConsumeToText(utf8_stringview css, F consumer)
  {
    ParserContext context {ParserMode::HTMLStandard};
    return ConsumeToText(css, consumer, context);
  }

  /// @brief A parser context in @p mode whose base URL is `https://example.com/dir/`.
  KRYS_NODISCARD inline ParserContext ContextWithBaseUrl(ParserMode mode = ParserMode::HTMLStandard)
  {
    return ParserContext {mode, net::Url(u8"https://example.com/dir/")};
  }
}
