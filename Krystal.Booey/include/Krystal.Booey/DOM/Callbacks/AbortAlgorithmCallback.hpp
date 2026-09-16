#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Any.hpp"
#include "Krystal.Core/Types/Func.hpp"
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

  public:
    AbortAlgorithmCallback(AnyCallback &&callback) noexcept
        : _id(std::rand()), _callback(krys::move(callback))
    {
    }

    template <typename Callable>
    AbortAlgorithmCallback(Callable &&callback) noexcept
        : _id(std::rand()), _callback(AnyCallback(krys::move(callback)))
    {
    }

    AbortAlgorithmCallback(const AbortAlgorithmCallback &other) noexcept
        : _id(other._id), _callback(other._callback)
    {
    }

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