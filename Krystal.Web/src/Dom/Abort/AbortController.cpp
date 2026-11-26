#include "Krystal.Web/Dom/Abort/AbortController.hpp"
#include "Krystal.Web/Dom/Abort/AbortAlgorithms.hpp"
#include "Krystal.Web/Dom/Abort/AbortSignal.hpp"

namespace krys::web
{
  AbortController::AbortController() noexcept
      // 1. Let signal be a new AbortSignal object.
      // 2. Set this's signal to signal.
      : _signal(CreateRef<AbortSignal>())
  {
  }

  AbortController::~AbortController() noexcept = default;

  void AbortController::Abort(Maybe<Exception> reason) noexcept
  {
    // The spec's signal abort on the controller's signal (AbortSignal::Abort is the static abort()).
    dom::AbortAlgorithms::SignalAbort(*this, krys::move(reason));
  }
}
