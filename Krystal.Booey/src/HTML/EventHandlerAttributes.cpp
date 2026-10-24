#include "Krystal.Booey/HTML/EventHandlerAttributes.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLFrameSetElement.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include <algorithm>
#include <array>

namespace krys::boo::html
{
  namespace
  {
    /// @see https://html.spec.whatwg.org/#window-reflecting-body-element-event-handler-set
    constexpr std::array<utf8_stringview, 6> WindowReflectingBodyElementEventHandlers {
      u8"blur", u8"error", u8"focus", u8"load", u8"resize", u8"scroll"};

#define KRYS_EVENT_HANDLER_NAME(Name, name) utf8_stringview(name),
    constexpr std::array WindowEventHandlerNames {KRYS_WINDOW_EVENT_HANDLERS(KRYS_EVENT_HANDLER_NAME)};
#undef KRYS_EVENT_HANDLER_NAME

    KRYS_NODISCARD bool IsReflectedOnWindow(dom::DOMStringAtom name) noexcept
    {
      auto view = name.View();
      return std::ranges::find(WindowReflectingBodyElementEventHandlers, view)
               != WindowReflectingBodyElementEventHandlers.end()
             || std::ranges::find(WindowEventHandlerNames, view) != WindowEventHandlerNames.end();
    }
  }

  dom::EventTarget *EventHandlerAlgorithms::DetermineTarget(dom::EventTarget &eventTarget,
                                                            dom::DOMStringAtom name) noexcept
  {
    // If eventTarget is not a body element or a frameset element, or name isn't the name of an attribute
    // of the WindowEventHandlers interface mixin or the window-reflecting body element event handler set,
    // then return eventTarget.
    auto *node = DynamicDowncast<dom::Node>(eventTarget);
    auto *element = node ? DynamicDowncast<HTMLElement>(*node) : nullptr;
    if (!element || !(Is<HTMLBodyElement>(*element) || Is<HTMLFrameSetElement>(*element))
        || !IsReflectedOnWindow(name))
    {
      return &eventTarget;
    }

    // If eventTarget's node document is not an active document, then return null. Otherwise return its
    // relevant global object.
    auto &document = element->NodeDocument();
    if (!document.Frame())
    {
      return nullptr;
    }
    return document.Window();
  }

  const EventHandlerCallback *EventHandlerAlgorithms::Get(const dom::EventTarget &eventTarget,
                                                          dom::DOMStringAtom name) noexcept
  {
    // Let eventTarget be the result of determining the target of an event handler; if it's null, return null.
    auto *target = DetermineTarget(const_cast<dom::EventTarget &>(eventTarget), name);
    return target ? target->EventHandlerValue(name) : nullptr;
  }

  void EventHandlerAlgorithms::Set(dom::EventTarget &eventTarget, dom::DOMStringAtom name,
                                   EventHandlerCallback &&value) noexcept
  {
    // Let eventTarget be the result of determining the target of an event handler; if it's null, return.
    if (auto *target = DetermineTarget(eventTarget, name))
    {
      target->SetEventHandlerValue(name, krys::move(value));
    }
  }
}
