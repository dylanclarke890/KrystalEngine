#pragma once

#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class VariableData : public RefCounted<VariableData>
  {
  private:
    CSSOMString _backingString;
    SmallList<Token> _tokens;
    const ParserContext _context;

    VariableData(const css::TokenRange &range, const ParserContext &context) noexcept;

  public:
    KRYS_NODISCARD static Ref<VariableData> Create(const TokenRange &range,
                                                      const ParserContext &context = {}) noexcept
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

    KRYS_NODISCARD const SmallList<Token> &Tokens() const noexcept
    {
      return _tokens;
    }

    bool operator==(const VariableData &other) const noexcept;

    KRYS_NODISCARD CSSOMString Serialise() const noexcept;

  private:
    template <typename CharacterType>
    void UpdateBackingStringsInTokens() noexcept;
  };
}
