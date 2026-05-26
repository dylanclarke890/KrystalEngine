#pragma once

#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Types/Any.hpp"
#include "Krystal.Lib/Types/Func.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  class AbortAlgorithmCallback
  {
    using AnyCallback = Func<void(const Any &reason)>;

  private:
    uint64 _id;
    AnyCallback _callback;

  public:
    AbortAlgorithmCallback(AnyCallback &&callback) noexcept
        : _id(std::rand()), _callback(Krys::Move(callback))
    {
    }

    template <typename Callable>
    AbortAlgorithmCallback(Callable &&callback) noexcept
        : _id(std::rand()), _callback(Krys::Func<void(const Any &)>(Krys::Move(callback)))
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