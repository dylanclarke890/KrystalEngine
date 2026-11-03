#pragma once

#include "Krystal.Booey/DOM/Types/DOMHighResTimeStamp.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  /// @brief What an idle callback is given: how long the idle period has left, or that its timeout passed.
  /// @see https://w3c.github.io/requestidlecallback/#the-idledeadline-interface
  class IdleDeadline final : public RefCounted<IdleDeadline>
  {
    KRYS_NON_COPY_MOVABLE(IdleDeadline);

  public:
    enum class DidTimeout : bool
    {
      No,
      Yes
    };

  private:
    /// @brief When the idle period ends, in real time.
    MonotonicTime::duration _deadline;
    DidTimeout _didTimeout;

  public:
    IdleDeadline(MonotonicTime::duration deadline, DidTimeout didTimeout) noexcept
        : _deadline(deadline), _didTimeout(didTimeout)
    {
    }

    KRYS_NODISCARD static Ref<IdleDeadline> Create(MonotonicTime::duration deadline,
                                                   DidTimeout didTimeout) noexcept
    {
      return CreateRef<IdleDeadline>(deadline, didTimeout);
    }

    /// @brief The milliseconds left in the idle period; 0 if it's over or the callback's timeout passed.
    /// @see https://w3c.github.io/requestidlecallback/#dom-idledeadline-timeremaining
    KRYS_NODISCARD DOMHighResTimeStamp TimeRemaining() const noexcept;

    /// @see https://w3c.github.io/requestidlecallback/#dom-idledeadline-didtimeout
    KRYS_NODISCARD bool DidTimeoutValue() const noexcept
    {
      return _didTimeout == DidTimeout::Yes;
    }
  };
}
