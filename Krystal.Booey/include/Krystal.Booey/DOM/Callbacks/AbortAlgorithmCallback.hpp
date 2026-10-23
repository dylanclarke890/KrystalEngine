#pragma once

#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Any.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#abortsignal-abort-algorithms
  class AbortAlgorithmCallback
  {
    using AnyCallback = Func<void(const Any &reason)>;

  private:
    uint64 _id;
    AnyCallback _callback;

    /// @brief A distinct identity for each algorithm, which copies share.
    KRYS_NODISCARD static uint64 NextId() noexcept
    {
      static uint64 nextId = 0;
      return ++nextId;
    }

  public:
    AbortAlgorithmCallback(AnyCallback &&callback) noexcept : _id(NextId()), _callback(krys::move(callback))
    {
    }

    template <typename Callable>
    requires(!SameType<remove_cvref_t<Callable>, AbortAlgorithmCallback>)
    AbortAlgorithmCallback(Callable &&callback) noexcept
        : _id(NextId()), _callback(AnyCallback(std::forward<Callable>(callback)))
    {
    }

    AbortAlgorithmCallback(const AbortAlgorithmCallback &other) noexcept = default;
    AbortAlgorithmCallback(AbortAlgorithmCallback &&other) noexcept = default;
    AbortAlgorithmCallback &operator=(const AbortAlgorithmCallback &other) noexcept = default;
    AbortAlgorithmCallback &operator=(AbortAlgorithmCallback &&other) noexcept = default;

    void operator()(const Any &reason) const noexcept
    {
      _callback(reason);
    }

    KRYS_NODISCARD bool operator==(const AbortAlgorithmCallback &other) const noexcept
    {
      return _id == other._id;
    }
  };
}