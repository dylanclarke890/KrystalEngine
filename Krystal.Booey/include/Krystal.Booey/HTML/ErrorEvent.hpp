#pragma once

#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/SerializedScriptValue.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#erroreventinit
  struct ErrorEventInit : dom::EventInit
  {
    dom::DOMString Message;
    dom::USVString Filename;
    uint32 Lineno {0};
    uint32 Colno {0};
    RefPtr<SerializedScriptValue> Error;
  };

  /// @brief An error reported to a window (`reportError()`), which the page can cancel before it reaches the
  /// console.
  /// @see https://html.spec.whatwg.org/#errorevent
  class ErrorEvent final : public dom::Event
  {
    friend class dom::EventFactory;
    using dictionary_type = ErrorEventInit;

  private:
    dom::DOMString _message;
    dom::USVString _filename;
    uint32 _lineno {0};
    uint32 _colno {0};
    RefPtr<SerializedScriptValue> _error;

  public:
    /// @see https://html.spec.whatwg.org/#dom-errorevent-errorevent
    ErrorEvent(dom::DOMStringAtom type, const ErrorEventInit &eventInitDict = {}) noexcept;

    /// @brief A trusted, cancelable `error` event.
    /// @brief A trusted error event of type @p type (`navigateerror`), which can't be canceled.
    KRYS_NODISCARD static Ref<ErrorEvent> Create(dom::DOMStringAtom type, dom::DOMString &&message,
                                                 dom::USVString &&filename, uint32 lineno, uint32 colno,
                                                 RefPtr<SerializedScriptValue> &&error) noexcept;

    KRYS_NODISCARD static Ref<ErrorEvent> Create(dom::DOMString &&message, dom::USVString &&filename,
                                                 uint32 lineno, uint32 colno,
                                                 RefPtr<SerializedScriptValue> &&error) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-errorevent-message
    KRYS_NODISCARD const dom::DOMString &Message() const noexcept
    {
      return _message;
    }

    /// @see https://html.spec.whatwg.org/#dom-errorevent-filename
    KRYS_NODISCARD const dom::USVString &Filename() const noexcept
    {
      return _filename;
    }

    /// @see https://html.spec.whatwg.org/#dom-errorevent-lineno
    KRYS_NODISCARD uint32 Lineno() const noexcept
    {
      return _lineno;
    }

    /// @see https://html.spec.whatwg.org/#dom-errorevent-colno
    KRYS_NODISCARD uint32 Colno() const noexcept
    {
      return _colno;
    }

    /// @see https://html.spec.whatwg.org/#dom-errorevent-error
    KRYS_NODISCARD SerializedScriptValue *Error() const noexcept
    {
      return _error.get();
    }

    KRYS_NODISCARD bool IsErrorEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::ErrorEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsErrorEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
