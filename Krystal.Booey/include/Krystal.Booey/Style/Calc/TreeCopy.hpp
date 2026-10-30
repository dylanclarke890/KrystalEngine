#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::style::calc
{
  struct Tree;

  // MARK: Copying

  /// @brief Makes a copy of the tree.
  KRYS_NODISCARD Tree Copy(const Tree &) noexcept;
}
