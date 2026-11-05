#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::css
{
  /// @brief Queues the mutation record for a change to an element's `style` attribute made through its inline
  /// style's CSSOM view. Scopes nest; only the outermost one reads the old value and queues the record, and
  /// only if `EnqueueMutationRecord` was called.
  class StyleAttributeMutationScope
  {
    KRYS_NON_COPY_MOVABLE(StyleAttributeMutationScope);

  private:
    RefPtr<dom::Element> _element;
    Maybe<dom::DOMString> _oldValue;
    bool _isCustomElement {false};
    bool _shouldDeliver {false};

  public:
    explicit StyleAttributeMutationScope(dom::Element *element) noexcept;
    ~StyleAttributeMutationScope() noexcept;

    /// @brief Queues the record when the outermost scope ends.
    void EnqueueMutationRecord() noexcept;
  };
}
