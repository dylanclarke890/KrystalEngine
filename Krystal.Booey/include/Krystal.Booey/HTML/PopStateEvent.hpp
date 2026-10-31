#pragma once

#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/HTML/SerializedScriptValue.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class History;
}

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#popstateeventinit
  struct PopStateEventInit : dom::EventInit
  {
    RefPtr<SerializedScriptValue> State;
    bool HasUAVisualTransition {false};
  };

  /// @brief `popstate`: the frame went to a history entry of its document (back, forward, or a fragment),
  /// with the entry's state.
  /// @see https://html.spec.whatwg.org/#popstateevent
  class PopStateEvent final : public dom::Event
  {
  private:
    RefPtr<SerializedScriptValue> _state;
    RefPtr<boo::History> _history;
    bool _hasUAVisualTransition {false};

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = PopStateEventInit;

    /// @see https://html.spec.whatwg.org/#dom-popstateevent-popstateevent
    PopStateEvent(dom::DOMStringAtom type, const PopStateEventInit &eventInitDict = {}) noexcept;
    ~PopStateEvent() noexcept override;

    /// @brief A trusted `popstate` with @p state, from @p history.
    KRYS_NODISCARD static Ref<PopStateEvent> Create(RefPtr<SerializedScriptValue> &&state,
                                                    boo::History *history) noexcept;

    /// @brief The entry's state; null for the spec's null.
    /// @see https://html.spec.whatwg.org/#dom-popstateevent-state
    KRYS_NODISCARD SerializedScriptValue *State() const noexcept
    {
      return _state.get();
    }

    KRYS_NODISCARD boo::History *GetHistory() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-popstateevent-hasuavisualtransition
    KRYS_NODISCARD bool HasUAVisualTransition() const noexcept
    {
      return _hasUAVisualTransition;
    }

    KRYS_NODISCARD bool IsPopStateEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::PopStateEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsPopStateEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
