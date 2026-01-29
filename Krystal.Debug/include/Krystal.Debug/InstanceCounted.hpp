#pragma once

#include "Krystal.Lib/Detection/Environment.hpp"
#include <atomic>
#include <cassert>
#include <cstddef>

// This class implements "instance count" management for regression test coverage.
// Since it adds runtime overhead to manage the count variables, the actual
// functionality of the class is limited to debug builds.
namespace Krys
{
  template <typename T>
  class InstanceCounted
  {
  public:
    struct Metrics
    {
      /// @brief The number of currently existing instances of the class.
      size_t Instances {0uz};

      /// @brief The total number of constructed instances of the class.
      size_t Constructed {0uz};

      /// @brief The total number of destructed instances of the class.
      size_t Destructed {0uz};

      /// @brief The total number of copied instances of the class.
      size_t Copied {0uz};

      /// @brief The total number of moved instances of the class.
      size_t Moved {0uz};
    };

  private:
    static inline std::atomic_size_t _instanceCount;
    static inline std::atomic_size_t _constructedCount;
    static inline std::atomic_size_t _destructedCount;
    static inline std::atomic_size_t _copiedCount;
    static inline std::atomic_size_t _movedCount;

  public:
    static Metrics GetMetrics() noexcept
    {
#if KRYS_ENV(DEV)
      return {
        .Instances = _instanceCount.load(),
        .Constructed = _constructedCount.load(),
        .Destructed = _destructedCount.load(),
        .Copied = _copiedCount.load(),
        .Moved = _movedCount.load(),
      };
#else
      return {};
#endif
    }

#if KRYS_ENV(DEV)
  protected:
    InstanceCounted() noexcept
    {
      _instanceCount++;
      _constructedCount++;
    }

    InstanceCounted(const InstanceCounted &) noexcept
    {
      _instanceCount++;
      _copiedCount++;
    }

    InstanceCounted(InstanceCounted &&) noexcept
    {
      _instanceCount++;
      _movedCount++;
    }

    ~InstanceCounted() noexcept
    {
      assert(_instanceCount);
      --_instanceCount;
      _destructedCount++;
    }
#endif
  };
}
