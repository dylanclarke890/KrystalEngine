#pragma once

#include "Krystal.HTML/CSS/Calc/CSSCalcValue.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaResolver.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumeric.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSUnevaluatedCalc.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  // MARK: - Comma
  KRYS_NODISCARD bool ConsumeComma(CSSTokenRange &tokens) noexcept;

  // MARK: - Slash
  KRYS_NODISCARD bool ConsumeSlash(CSSTokenRange &tokens) noexcept;

  // MARK: - Function
  // NOTE: ConsumeFunction expects the range starts with a FunctionToken.
  KRYS_NODISCARD CSSTokenRange ConsumeFunction(CSSTokenRange &tokens) noexcept;

  KRYS_NODISCARD Maybe<CSSTokenRange> ConsumeArgument(CSSTokenRange &tokens, size_t index) noexcept;
}