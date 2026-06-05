#include "Krystal.HTML/DOM/AbortController.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"

namespace Krys::HTML
{
  AbortController::AbortController() noexcept : _signal(CreateRef<AbortSignal>())
  {
  }

  void AbortController::Abort(Maybe<::Krys::Any> reason) noexcept
  {
    if (_signal)
    {
      _signal->Abort(reason);
    }
  }
}