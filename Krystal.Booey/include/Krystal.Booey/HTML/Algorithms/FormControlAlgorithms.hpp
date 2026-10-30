#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::html
{
  class HTMLOptionElement;
  class HTMLSelectedContentElement;
  class HTMLSelectElement;

  /// @brief The selectedcontent algorithms of selects and options.
  class FormControlAlgorithms
  {
  public:
#pragma region HTMLSelectedContentElement

    /// @see https://html.spec.whatwg.org/#select-enabled-selectedcontent
    KRYS_NODISCARD static HTMLSelectedContentElement *
      GetEnabledSelectedContent(HTMLSelectElement &select) noexcept;

    /// @see https://html.spec.whatwg.org/#clone-an-option-into-a-selectedcontent
    static void CloneOptionIntoSelectedContent(HTMLOptionElement &option,
                                               HTMLSelectedContentElement &selectedcontent) noexcept;
#pragma endregion

#pragma region HTMLOptionElement

    /// @see https://html.spec.whatwg.org/#maybe-clone-an-option-into-selectedcontent
    static void MaybeCloneOptionIntoSelectedContent(HTMLOptionElement &option) noexcept;

#pragma endregion
  };
}