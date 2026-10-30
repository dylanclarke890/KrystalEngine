#pragma once

#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/SerializedScriptValue.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class LocalDOMWindow;
}

namespace krys::boo::html
{
  /// @note There are no message ports: `ports` is always empty.
  /// @see https://html.spec.whatwg.org/#messageeventinit
  struct MessageEventInit : dom::EventInit
  {
    RefPtr<SerializedScriptValue> Data;
    dom::USVString Origin;
    dom::DOMString LastEventId;
    /// @brief The window that sent the message, if any.
    RefPtr<LocalDOMWindow> Source;
  };

  /// @brief A message posted to a window (`postMessage()`).
  /// @see https://html.spec.whatwg.org/#messageevent
  class MessageEvent final : public dom::Event
  {
    friend class dom::EventFactory;
    using dictionary_type = MessageEventInit;

  private:
    RefPtr<SerializedScriptValue> _data;
    dom::USVString _origin;
    dom::DOMString _lastEventId;
    RefPtr<LocalDOMWindow> _source;

  public:
    /// @see https://html.spec.whatwg.org/#dom-messageevent-messageevent
    MessageEvent(dom::DOMStringAtom type, const MessageEventInit &eventInitDict = {}) noexcept;
    ~MessageEvent() noexcept override;

    /// @brief A trusted `message` event.
    KRYS_NODISCARD static Ref<MessageEvent> Create(RefPtr<SerializedScriptValue> &&data,
                                                   dom::USVString &&origin,
                                                   RefPtr<LocalDOMWindow> &&source) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-messageevent-data
    KRYS_NODISCARD SerializedScriptValue *Data() const noexcept
    {
      return _data.get();
    }

    /// @see https://html.spec.whatwg.org/#dom-messageevent-origin
    KRYS_NODISCARD const dom::USVString &Origin() const noexcept
    {
      return _origin;
    }

    /// @see https://html.spec.whatwg.org/#dom-messageevent-lasteventid
    KRYS_NODISCARD const dom::DOMString &LastEventId() const noexcept
    {
      return _lastEventId;
    }

    /// @see https://html.spec.whatwg.org/#dom-messageevent-source
    KRYS_NODISCARD LocalDOMWindow *Source() const noexcept;

    KRYS_NODISCARD bool IsMessageEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::MessageEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsMessageEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
