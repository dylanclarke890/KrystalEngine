#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  enum class PaginationMode : uint8
  {
    Unpaginated,
    LeftToRightPaginated,
    RightToLeftPaginated,
    TopToBottomPaginated,
    BottomToTopPaginated
  };

  /// @brief How a view lays its content out in pages.
  struct Pagination
  {
    friend bool operator==(const Pagination &, const Pagination &) = default;

    PaginationMode Mode {PaginationMode::Unpaginated};
    bool BehavesLikeColumns {false};
    uint32 PageLength {0};
    uint32 Gap {0};
  };
}
