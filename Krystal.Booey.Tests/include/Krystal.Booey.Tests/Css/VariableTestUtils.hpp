#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Css/Parser/InputStream.hpp"
#include "Krystal.Booey/Css/Parser/ParserContext.hpp"
#include "Krystal.Booey/Css/Parser/Tokenizer.hpp"
#include "Krystal.Booey/Css/Values/VariableData.hpp"
#include "Krystal.Booey/Css/Values/VariableReferenceValue.hpp"

namespace krys::boo::css::tests
{
  /// @brief The tokens of @p css as variable data, parsed with @p context.
  KRYS_NODISCARD inline Ref<VariableData>
    MakeVariableData(utf8_stringview css, const ParserContext &context = StrictParserContext())
  {
    if (css.empty())
    {
      return VariableData::Create(css::TokenRange(Span<const Token> {}), context);
    }
    InputStream input {CSSOMString(css)};
    Tokenizer tokenizer {input};
    REQUIRE(tokenizer.PumpTokenizer());
    return VariableData::Create(tokenizer.Tokens(), context);
  }

  /// @brief A value referencing variables, from @p css.
  KRYS_NODISCARD inline Ref<VariableReferenceValue> MakeVariableReference(utf8_stringview css)
  {
    return VariableReferenceValue::Create(MakeVariableData(css));
  }
}
