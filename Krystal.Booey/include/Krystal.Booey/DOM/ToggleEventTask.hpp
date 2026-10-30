#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Element;

  enum class ToggleState : bool
  {
    Closed,
    Open,
  };

  struct ToggleEventData
  {
    ToggleState OldState;
    ToggleState NewState;
    RefPtr<Element> Source;
  };

  /// @brief An element's `toggle` event task: toggling it again before the task runs updates the queued
  /// event's new state (and source), keeping its old state, so one event reports both changes.
  /// @see https://html.spec.whatwg.org/#queue-a-popover-toggle-event-task
  /// @see https://html.spec.whatwg.org/#queue-a-dialog-toggle-event-task
  class ToggleEventTask final : public RefCounted<ToggleEventTask>
  {
    KRYS_NON_COPY_MOVABLE(ToggleEventTask);

  private:
    WeakPtr<Element> _element;
    Maybe<ToggleEventData> _data;

  public:
    explicit ToggleEventTask(Element &element) noexcept;

    KRYS_NODISCARD static Ref<ToggleEventTask> Create(Element &element) noexcept
    {
      return CreateRef<ToggleEventTask>(element);
    }

    /// @brief Queues the `toggle` event from @p oldState to @p newState, invoked by @p source, or updates the
    /// queued one.
    void Queue(ToggleState oldState, ToggleState newState, Element *source = nullptr) noexcept;
  };
}
