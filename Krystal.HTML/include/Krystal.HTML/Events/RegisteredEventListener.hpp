#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Events/EventListener.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#concept-event-listener
  class RegisteredEventListener : public RefCounted<RegisteredEventListener>
  {
  public:
    struct Options
    {
      bool Capture : 1 {false};
      bool Passive : 1 {false};
      bool Once : 1 {false};
      bool TrustedOnly : 1 {false};
    };

  private:
    const DOMStringAtom _type;
    bool _useCapture : 1;
    bool _isPassive : 1;
    bool _isOnce : 1;
    bool _wasRemoved : 1;
    bool _trustedOnly : 1;
    const RefPtr<EventListener> _callback;

  public:
    RegisteredEventListener(const DOMStringAtom type, const Options &options,
                            RefPtr<EventListener> &&callback) noexcept
        : _type(type), _useCapture(options.Capture), _isPassive(options.Passive), _isOnce(options.Once),
          _trustedOnly(options.TrustedOnly), _callback(Krys::Move(callback))
    {
    }

    KRYS_NODISCARD DOMStringAtom GetType() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD EventListener &Callback() const noexcept
    {
      return *_callback.get();
    }

    KRYS_NODISCARD bool UseCapture() const noexcept
    {
      return _useCapture;
    }

    KRYS_NODISCARD bool IsPassive() const noexcept
    {
      return _isPassive;
    }

    KRYS_NODISCARD bool IsOnce() const noexcept
    {
      return _isOnce;
    }

    KRYS_NODISCARD bool TrustedOnly() const noexcept
    {
      return _trustedOnly;
    }

    KRYS_NODISCARD bool WasRemoved() const noexcept
    {
      return _wasRemoved;
    }

    KRYS_NODISCARD void MarkAsRemoved() noexcept
    {
      _wasRemoved = true;
    }
  };
}