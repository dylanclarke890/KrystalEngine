#include "Krystal.Booey/DOM/AbortController.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"

namespace krys::boo::dom
{
  AbortController::AbortController() noexcept : _signal(CreateRef<AbortSignal>())
  {
  }

  void AbortController::Abort(Maybe<::krys::Any> reason) noexcept
  {
    if (_signal)
    {
      _signal->Abort(reason);
    }
  }
}