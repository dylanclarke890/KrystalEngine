#pragma once

#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief Increments a nesting level for the lifetime of the incrementer.
  class NestingLevelIncrementer
  {
    KRYS_NON_COPYABLE(NestingLevelIncrementer);

  private:
    uint32 *_nestingLevel;

  public:
    explicit NestingLevelIncrementer(uint32 &nestingLevel) noexcept : _nestingLevel(&nestingLevel)
    {
      ++(*_nestingLevel);
    }

    ~NestingLevelIncrementer() noexcept
    {
      --(*_nestingLevel);
    }
  };
}
