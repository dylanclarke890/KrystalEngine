#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::html
{
  /// @brief Where a task comes from. Tasks from the same source run in the order they were queued.
  /// @see https://html.spec.whatwg.org/#generic-task-sources
  enum class TaskSource : uint8
  {
    /// @see https://html.spec.whatwg.org/#dom-manipulation-task-source
    DOMManipulation,
    /// @see https://html.spec.whatwg.org/#user-interaction-task-source
    UserInteraction,
    /// @see https://html.spec.whatwg.org/#networking-task-source
    Networking,
    /// @see https://html.spec.whatwg.org/#navigation-and-traversal-task-source
    NavigationAndTraversal,
    /// @see https://html.spec.whatwg.org/#rendering-task-source
    Rendering,
    /// @see https://html.spec.whatwg.org/#timer-task-source
    Timer,
    /// @see https://w3c.github.io/requestidlecallback/#start-an-idle-period-algorithm
    IdleTask,
    /// @see https://html.spec.whatwg.org/#posted-message-task-source
    PostedMessage,
    /// @brief The user agent's own asynchronous work.
    InternalAsyncTask,
    /// @see https://w3c.github.io/gamepad/#dfn-gamepad-task-source
    Gamepad,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::TaskSource, 10uz);
