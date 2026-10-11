#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::css::calc
{
  struct AnchorSide;
  struct Tree;

  /// @brief Makes a copy of the tree.
  KRYS_NODISCARD Tree Copy(const Tree &) noexcept;

  /// @brief Makes a copy of the anchor side.
  KRYS_NODISCARD AnchorSide Copy(const AnchorSide &) noexcept;
}