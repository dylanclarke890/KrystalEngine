#pragma once

#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief The tokens of a custom property value or a value containing `var()`/`env()`, with the context
  /// they were parsed in.
  class VariableData : public RefCounted<VariableData>
  {
  private:
    List<Token> _tokens;
    const ParserContext _context;

    VariableData(const css::TokenRange &range, const ParserContext &context) noexcept;

  public:
    KRYS_NODISCARD static Ref<VariableData>
      Create(const css::TokenRange &range, const ParserContext &context = StrictParserContext()) noexcept
    {
      return AdoptRef(*new VariableData(range, context));
    }

    KRYS_NODISCARD css::TokenRange TokenRange() const noexcept
    {
      return css::TokenRange(_tokens);
    }

    KRYS_NODISCARD const ParserContext &Context() const noexcept
    {
      return _context;
    }

    KRYS_NODISCARD const List<Token> &Tokens() const noexcept
    {
      return _tokens;
    }

    KRYS_NODISCARD bool operator==(const VariableData &other) const noexcept;

    KRYS_NODISCARD CSSOMString Serialise() const noexcept;
  };
}
