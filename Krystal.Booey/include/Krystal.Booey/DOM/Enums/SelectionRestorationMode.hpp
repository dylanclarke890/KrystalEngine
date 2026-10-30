#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::dom
{
  /// @brief What focusing an element that has a selection of its own (a text control) does with it.
  enum class SelectionRestorationMode : uint8
  {
    RestoreOrSelectAll,
    SelectAll,
    PlaceCaretAtStart,
  };
}
