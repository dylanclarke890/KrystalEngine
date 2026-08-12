#pragma once

#include "Krystal.HTML/CSS/Calc/CSSCalcValue.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys::HTML
{
  void UnevaluatedCalcAddRef(RawPtr<CSSCalcValue>) noexcept;

  void UnevaluatedCalcSubRef(RawPtr<CSSCalcValue>) noexcept;
}