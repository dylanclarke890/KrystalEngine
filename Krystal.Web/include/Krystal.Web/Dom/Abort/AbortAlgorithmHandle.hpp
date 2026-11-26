#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::web
{
  class AbortSignal;

  /// @brief Owns an abort algorithm added to a signal: removes it when it's destroyed, unless it was released
  /// to its signal.
  /// @note Removing an algorithm that's already gone (run by the signal's abort, or dropped with the signal)
  /// does nothing. The handle doesn't hold the signal alive.
  /// @see https://dom.spec.whatwg.org/#abortsignal-abort-algorithms
  class KRYS_NODISCARD AbortAlgorithmHandle
  {
    KRYS_NON_COPYABLE(AbortAlgorithmHandle);

  private:
    WeakPtr<AbortSignal> _signal;
    uint64 _id {0};

  public:
    /// @brief A handle that owns no algorithm.
    AbortAlgorithmHandle() noexcept = default;

    AbortAlgorithmHandle(AbortSignal &signal, uint64 id) noexcept;

    AbortAlgorithmHandle(AbortAlgorithmHandle &&other) noexcept;

    AbortAlgorithmHandle &operator=(AbortAlgorithmHandle &&other) noexcept;

    ~AbortAlgorithmHandle() noexcept;

    /// @brief Removes the algorithm now, if the signal still has it; the handle then owns none.
    /// @see https://dom.spec.whatwg.org/#abortsignal-remove
    void Remove() noexcept;

    /// @brief Gives the algorithm up to its signal, which keeps it until it aborts; the handle then owns
    /// none.
    void Release() noexcept;

    /// @brief Whether the handle owns an algorithm that its signal still has.
    KRYS_NODISCARD bool IsActive() const noexcept;

    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return IsActive();
    }
  };
}
