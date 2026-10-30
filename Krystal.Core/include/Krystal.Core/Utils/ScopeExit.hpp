#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include <utility>

namespace krys
{
  /// @brief Calls a function when it goes out of scope, unless released: an alternative to repeating the same
  /// steps at all of a block's exit points.
  template <typename TExitFunction>
  class ScopeExit
  {
    KRYS_NON_COPY_MOVABLE(ScopeExit);

  private:
    TExitFunction _exitFunction;
    bool _execute {true};

  public:
    template <typename TFunction>
    explicit ScopeExit(TFunction &&exitFunction) noexcept
        : _exitFunction(std::forward<TFunction>(exitFunction))
    {
    }

    ~ScopeExit() noexcept
    {
      if (_execute)
      {
        _exitFunction();
      }
    }

    /// @brief The function won't be called.
    void Release() noexcept
    {
      _execute = false;
    }
  };

  /// @brief A `ScopeExit` calling @p exitFunction.
  template <typename TExitFunction>
  KRYS_NODISCARD ScopeExit<std::decay_t<TExitFunction>> MakeScopeExit(TExitFunction &&exitFunction) noexcept
  {
    return ScopeExit<std::decay_t<TExitFunction>>(std::forward<TExitFunction>(exitFunction));
  }
}
