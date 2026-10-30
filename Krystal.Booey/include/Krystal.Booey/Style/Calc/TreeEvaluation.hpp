#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::style
{
  struct ZoomFactor;
  struct ZoomNeeded;
}

namespace krys::boo::style::calc
{
  struct Tree;

  // MARK: Evaluation.

  KRYS_NODISCARD double Evaluate(const Tree &, double percentResolutionLength, const ZoomFactor &) noexcept;
  KRYS_NODISCARD double Evaluate(const Tree &, double percentResolutionLength, const ZoomNeeded &) noexcept;
}
