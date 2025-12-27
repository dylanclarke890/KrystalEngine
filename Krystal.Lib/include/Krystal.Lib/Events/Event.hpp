#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/StringId.hpp"

namespace Krys
{
  using EventType = StringId;

/// @brief Convenience macro for setting up the event type for a derived `Event`.
/// @param eventTypeName Must be a string literal. Must be unique across events.
#define KRYS_EVENT_CLASS_TYPE(eventTypeName)                                                                 \
  KRYS_NODISCARD static EventType GetStaticType() noexcept                                                   \
  {                                                                                                          \
    return KRYS_SID(eventTypeName);                                                                          \
  }                                                                                                          \
  KRYS_NODISCARD virtual EventType GetEventType() const noexcept override                                    \
  {                                                                                                          \
    return GetStaticType();                                                                                  \
  }                                                                                                          \
  KRYS_NODISCARD virtual string GetEventTypeName() const noexcept override                                   \
  {                                                                                                          \
    return eventTypeName;                                                                                    \
  }

  /// @brief Base event class.
  ///
  /// Custom events can easily be created by extending this class and specifying it's `EventType`.
  /// A macro has been provided to make this even easier, see `KRYS_EVENT_CLASS_TYPE()`.
  class Event
  {
  public:
    virtual ~Event() = default;

    /// @brief Gets this instance's `EventType`.
    KRYS_NODISCARD virtual EventType GetEventType() const noexcept = 0;

    KRYS_NODISCARD virtual string GetEventTypeName() const noexcept = 0;

  protected:
    /// @brief Constructs an `Event`.
    Event() noexcept = default;
  };
}