#include "Krystal.HTML/Events/EventDispatcher.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Events/Event.hpp"
#include "Krystal.HTML/Events/EventTarget.hpp"

namespace Krys::HTML
{
  ExceptionOr<bool> EventDispatcher::DispatchToTarget(Event &event, EventTarget &target,
                                                      bool legacyTargetOverrideFlag,
                                                      bool legacyOutputDidListenersThrowFlag) noexcept
  {
    event._dispatched = true;

    auto *targetOverride = &target;
    if (legacyTargetOverrideFlag)
    {
      // legacy target override flag is only used by HTML and only when target is a Window object.
      assert(target.IsWindow());

      // SPEC-VIOLATION(DOM, HTML): We don't implement Window.
      // Let targetOverride be target, if legacy target override flag is not given, and target’s associated
      // Document otherwise.
    }

    RawPtr<EventTarget> activationTarget = nullptr;
    RawPtr<EventTarget> relatedTarget = event.RelatedTarget();
    bool clearTargets = false;

    // TODO(IMPL): https://dom.spec.whatwg.org/#concept-event-dispatch
    return false;
  }
}