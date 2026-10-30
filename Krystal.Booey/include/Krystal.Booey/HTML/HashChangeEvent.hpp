#pragma once

#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#hashchangeeventinit
  struct HashChangeEventInit : dom::EventInit
  {
    dom::USVString OldURL;
    dom::USVString NewURL;
  };

  /// @brief The `hashchange` event: the document's URL changed in its fragment.
  /// @see https://html.spec.whatwg.org/#hashchangeevent
  class HashChangeEvent final : public dom::Event
  {
    friend class dom::EventFactory;
    using dictionary_type = HashChangeEventInit;

  private:
    dom::USVString _oldURL;
    dom::USVString _newURL;

  public:
    /// @see https://html.spec.whatwg.org/#dom-hashchangeevent-hashchangeevent
    HashChangeEvent(dom::DOMStringAtom type, const HashChangeEventInit &eventInitDict = {}) noexcept;

    /// @brief A trusted `hashchange` event.
    KRYS_NODISCARD static Ref<HashChangeEvent> Create(dom::USVString &&oldURL,
                                                      dom::USVString &&newURL) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hashchangeevent-oldurl
    KRYS_NODISCARD const dom::USVString &OldURL() const noexcept
    {
      return _oldURL;
    }

    /// @see https://html.spec.whatwg.org/#dom-hashchangeevent-newurl
    KRYS_NODISCARD const dom::USVString &NewURL() const noexcept
    {
      return _newURL;
    }

    KRYS_NODISCARD bool IsHashChangeEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HashChangeEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsHashChangeEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
