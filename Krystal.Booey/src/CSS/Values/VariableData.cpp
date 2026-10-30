#include "Krystal.Booey/CSS/Values/VariableData.hpp"

namespace krys::boo::css
{
  VariableData::VariableData(const css::TokenRange &range, const ParserContext &context) noexcept
      : _tokens(range.Tokens().begin(), range.Tokens().end()), _context(context)
  {
  }

  bool VariableData::operator==(const VariableData &other) const noexcept
  {
    return Tokens() == other.Tokens();
  }

  CSSOMString VariableData::Serialise() const noexcept
  {
    return TokenRange().Serialise(TokenSerialisationMode::CustomProperty);
  }
}
