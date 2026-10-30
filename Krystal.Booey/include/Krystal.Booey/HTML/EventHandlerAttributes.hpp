#pragma once

#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo::dom
{
  class Event;
}

namespace krys::boo::html
{
  /// @brief An event handler's value: a C++ function called with each event of its type.
  /// @note Script's event handlers return a value that can cancel the event; a C++ handler calls
  /// `PreventDefault()` instead.
  /// @see https://html.spec.whatwg.org/#eventhandler
  using EventHandlerCallback = Func<void(dom::Event &)>;

  /// @brief The event handler IDL attributes' getting and setting steps.
  /// @see https://html.spec.whatwg.org/#event-handler-attributes
  class EventHandlerAlgorithms
  {
  public:
    /// @brief Where the event handler @p name of @p eventTarget is: the target itself, or, for the body and
    /// frameset elements' window-reflecting and `WindowEventHandlers` handlers, their document's window
    /// (null if the document isn't fully active).
    /// @see https://html.spec.whatwg.org/#determining-the-target-of-an-event-handler
    KRYS_NODISCARD static dom::EventTarget *DetermineTarget(dom::EventTarget &eventTarget,
                                                            dom::DOMStringAtom name) noexcept;

    /// @brief The value of @p eventTarget's event handler @p name; null if it has none.
    /// @see https://html.spec.whatwg.org/#getting-the-current-value-of-the-event-handler
    KRYS_NODISCARD static const EventHandlerCallback *Get(const dom::EventTarget &eventTarget,
                                                          dom::DOMStringAtom name) noexcept;

    /// @brief Sets @p eventTarget's event handler @p name to @p value, or deactivates it if @p value is
    /// empty.
    /// @see https://html.spec.whatwg.org/#event-handler-idl-attributes
    static void Set(dom::EventTarget &eventTarget, dom::DOMStringAtom name,
                    EventHandlerCallback &&value) noexcept;
  };
}

/// @brief The event handlers of every HTML, SVG and MathML element, document and window.
/// @see https://html.spec.whatwg.org/#globaleventhandlers
#define KRYS_GLOBAL_EVENT_HANDLERS(X)                                                                        \
  X(Abort, u8"abort")                                                                                        \
  X(AnimationCancel, u8"animationcancel")                                                                    \
  X(AnimationEnd, u8"animationend")                                                                          \
  X(AnimationIteration, u8"animationiteration")                                                              \
  X(AnimationStart, u8"animationstart")                                                                      \
  X(AuxClick, u8"auxclick")                                                                                  \
  X(BeforeInput, u8"beforeinput")                                                                            \
  X(BeforeMatch, u8"beforematch")                                                                            \
  X(BeforeToggle, u8"beforetoggle")                                                                          \
  X(Blur, u8"blur")                                                                                          \
  X(Cancel, u8"cancel")                                                                                      \
  X(CanPlay, u8"canplay")                                                                                    \
  X(CanPlayThrough, u8"canplaythrough")                                                                      \
  X(Change, u8"change")                                                                                      \
  X(Click, u8"click")                                                                                        \
  X(Close, u8"close")                                                                                        \
  X(Command, u8"command")                                                                                    \
  X(ContextLost, u8"contextlost")                                                                            \
  X(ContextMenu, u8"contextmenu")                                                                            \
  X(ContextRestored, u8"contextrestored")                                                                    \
  X(CueChange, u8"cuechange")                                                                                \
  X(DblClick, u8"dblclick")                                                                                  \
  X(Drag, u8"drag")                                                                                          \
  X(DragEnd, u8"dragend")                                                                                    \
  X(DragEnter, u8"dragenter")                                                                                \
  X(DragLeave, u8"dragleave")                                                                                \
  X(DragOver, u8"dragover")                                                                                  \
  X(DragStart, u8"dragstart")                                                                                \
  X(Drop, u8"drop")                                                                                          \
  X(DurationChange, u8"durationchange")                                                                      \
  X(Emptied, u8"emptied")                                                                                    \
  X(Ended, u8"ended")                                                                                        \
  X(Error, u8"error")                                                                                        \
  X(Focus, u8"focus")                                                                                        \
  X(FormData, u8"formdata")                                                                                  \
  X(Input, u8"input")                                                                                        \
  X(Invalid, u8"invalid")                                                                                    \
  X(KeyDown, u8"keydown")                                                                                    \
  X(KeyPress, u8"keypress")                                                                                  \
  X(KeyUp, u8"keyup")                                                                                        \
  X(Load, u8"load")                                                                                          \
  X(LoadedData, u8"loadeddata")                                                                              \
  X(LoadedMetadata, u8"loadedmetadata")                                                                      \
  X(LoadStart, u8"loadstart")                                                                                \
  X(MouseDown, u8"mousedown")                                                                                \
  X(MouseEnter, u8"mouseenter")                                                                              \
  X(MouseLeave, u8"mouseleave")                                                                              \
  X(MouseMove, u8"mousemove")                                                                                \
  X(MouseOut, u8"mouseout")                                                                                  \
  X(MouseOver, u8"mouseover")                                                                                \
  X(MouseUp, u8"mouseup")                                                                                    \
  X(Pause, u8"pause")                                                                                        \
  X(Play, u8"play")                                                                                          \
  X(Playing, u8"playing")                                                                                    \
  X(Progress, u8"progress")                                                                                  \
  X(RateChange, u8"ratechange")                                                                              \
  X(Reset, u8"reset")                                                                                        \
  X(Resize, u8"resize")                                                                                      \
  X(Scroll, u8"scroll")                                                                                      \
  X(ScrollEnd, u8"scrollend")                                                                                \
  X(SecurityPolicyViolation, u8"securitypolicyviolation")                                                    \
  X(Seeked, u8"seeked")                                                                                      \
  X(Seeking, u8"seeking")                                                                                    \
  X(Select, u8"select")                                                                                      \
  X(SelectionChange, u8"selectionchange")                                                                    \
  X(SelectStart, u8"selectstart")                                                                            \
  X(SlotChange, u8"slotchange")                                                                              \
  X(Stalled, u8"stalled")                                                                                    \
  X(Submit, u8"submit")                                                                                      \
  X(Suspend, u8"suspend")                                                                                    \
  X(TimeUpdate, u8"timeupdate")                                                                              \
  X(Toggle, u8"toggle")                                                                                      \
  X(TransitionCancel, u8"transitioncancel")                                                                  \
  X(TransitionEnd, u8"transitionend")                                                                        \
  X(TransitionRun, u8"transitionrun")                                                                        \
  X(TransitionStart, u8"transitionstart")                                                                    \
  X(VolumeChange, u8"volumechange")                                                                          \
  X(Waiting, u8"waiting")                                                                                    \
  X(Wheel, u8"wheel")

/// @brief The event handlers of the window, which the body and frameset elements reflect.
/// @see https://html.spec.whatwg.org/#windoweventhandlers
#define KRYS_WINDOW_EVENT_HANDLERS(X)                                                                        \
  X(AfterPrint, u8"afterprint")                                                                              \
  X(BeforePrint, u8"beforeprint")                                                                            \
  X(BeforeUnload, u8"beforeunload")                                                                          \
  X(GamepadConnected, u8"gamepadconnected")                                                                  \
  X(GamepadDisconnected, u8"gamepaddisconnected")                                                            \
  X(HashChange, u8"hashchange")                                                                              \
  X(LanguageChange, u8"languagechange")                                                                      \
  X(Message, u8"message")                                                                                    \
  X(MessageError, u8"messageerror")                                                                          \
  X(Offline, u8"offline")                                                                                    \
  X(Online, u8"online")                                                                                      \
  X(PageHide, u8"pagehide")                                                                                  \
  X(PageReveal, u8"pagereveal")                                                                              \
  X(PageShow, u8"pageshow")                                                                                  \
  X(PageSwap, u8"pageswap")                                                                                  \
  X(PopState, u8"popstate")                                                                                  \
  X(RejectionHandled, u8"rejectionhandled")                                                                  \
  X(Storage, u8"storage")                                                                                    \
  X(UnhandledRejection, u8"unhandledrejection")                                                              \
  X(Unload, u8"unload")

/// @brief The event handlers of documents and elements.
/// @see https://html.spec.whatwg.org/#documentandelementeventhandlers
#define KRYS_DOCUMENT_AND_ELEMENT_EVENT_HANDLERS(X)                                                          \
  X(Copy, u8"copy")                                                                                          \
  X(Cut, u8"cut")                                                                                            \
  X(Paste, u8"paste")

/// @brief Declares the event handler IDL attribute `on<name>`: `On<Name>()` and `SetOn<Name>()`.
#define KRYS_DECLARE_EVENT_HANDLER(Name, name)                                                               \
  /** @see https://html.spec.whatwg.org/#event-handler-idl-attributes */                                     \
  KRYS_NODISCARD const ::krys::boo::html::EventHandlerCallback *On##Name() const noexcept                    \
  {                                                                                                          \
    return ::krys::boo::html::EventHandlerAlgorithms::Get(*this, ::krys::boo::dom::DOMStringAtom(name));     \
  }                                                                                                          \
  /** @see https://html.spec.whatwg.org/#event-handler-idl-attributes */                                     \
  void SetOn##Name(::krys::boo::html::EventHandlerCallback &&callback) noexcept                              \
  {                                                                                                          \
    ::krys::boo::html::EventHandlerAlgorithms::Set(*this, ::krys::boo::dom::DOMStringAtom(name),             \
                                                   ::krys::move(callback));                                  \
  }
