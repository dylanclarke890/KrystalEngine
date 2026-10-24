#include "Krystal.Booey/DOM/IdleDeadline.hpp"
#include <chrono>

namespace krys::boo::dom
{
  double IdleDeadline::TimeRemaining() const noexcept
  {
    if (_didTimeout == DidTimeout::Yes)
    {
      return 0;
    }
    auto remaining = _deadline - MonotonicTime::Now();
    if (remaining <= MonotonicTime::duration::zero())
    {
      return 0;
    }
    return std::chrono::duration<double, std::milli>(remaining).count();
  }
}
