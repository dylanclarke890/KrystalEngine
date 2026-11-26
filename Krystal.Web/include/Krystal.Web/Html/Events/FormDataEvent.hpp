#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"

namespace krys::web::html
{
  class DOMFormData;
}

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#formdataeventinit
  struct FormDataEventInit : EventInit
  {
    RefPtr<html::DOMFormData> FormData;
  };

  /// @brief The `formdata` event, fired at a form with the entry list it's about to submit, which listeners
  /// can change.
  /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#the-formdataevent-interface
  class FormDataEvent final : public Event
  {
  private:
    Ref<html::DOMFormData> _formData;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = FormDataEventInit;

    /// @see https://html.spec.whatwg.org/#dom-formdataevent-formdataevent
    FormDataEvent(DOMStringAtom type, const FormDataEventInit &eventInitDict) noexcept;

    /// @brief A trusted event of type @p type with @p formData.
    KRYS_NODISCARD static Ref<FormDataEvent> Create(const DOMStringAtom &type, bool canBubble,
                                                    bool isCancelable, bool isComposed,
                                                    Ref<html::DOMFormData> &&formData) noexcept;

    /// @see
    /// https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-formdataevent-formdata
    KRYS_NODISCARD html::DOMFormData &FormData() const noexcept
    {
      return *_formData;
    }

    KRYS_NODISCARD bool IsFormDataEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::FormDataEvent)
  KRYS_NODISCARD static bool IsType(const krys::web::Event &event) noexcept
  {
    return event.IsFormDataEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
