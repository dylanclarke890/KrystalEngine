#pragma once

#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  /// @brief The text an edit is about to insert, dispatched at the editing host before the edit so that a
  /// text control can change it (to keep within its maximum length, or drop line breaks a field can't hold).
  /// Pages never see it.
  class BeforeTextInsertedEvent final : public Event
  {
  private:
    DOMString _text;

  public:
    explicit BeforeTextInsertedEvent(DOMStringView text) noexcept;
    ~BeforeTextInsertedEvent() noexcept override;

    KRYS_NODISCARD static Ref<BeforeTextInsertedEvent> Create(DOMStringView text) noexcept;

    KRYS_NODISCARD const DOMString &Text() const noexcept
    {
      return _text;
    }

    void SetText(DOMString &&text) noexcept
    {
      _text = krys::move(text);
    }

    KRYS_NODISCARD bool IsBeforeTextInsertedEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::BeforeTextInsertedEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsBeforeTextInsertedEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
