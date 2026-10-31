#include "Krystal.Booey/DOM/AbortController.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/AbortAlgorithms.hpp"

namespace krys::boo::dom
{
  AbortController::AbortController() noexcept
      // 1. Let signal be a new AbortSignal object.
      // 2. Set this's signal to signal.
      : _signal(CreateRef<AbortSignal>())
  {
  }

  void AbortController::Abort(Maybe<::krys::Any> reason) noexcept
  {
    // The spec's signal abort on the controller's signal (AbortSignal::Abort is the static abort()).
    AbortAlgorithms::SignalAbort(*this, krys::move(reason));
  }
}