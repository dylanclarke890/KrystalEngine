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

    /// @brief Updates the enabled selectedcontent elements in a select without multiple to show its first
    /// selected option.
    /// @see https://html.spec.whatwg.org/#update-a-select's-descendant-selectedcontent-elements
    static void UpdateDescendantSelectedContents(HTMLSelectElement &select) noexcept;

    /// @see https://html.spec.whatwg.org/#update-a-selectedcontent
    static void UpdateSelectedContent(HTMLSelectElement &select,
                                      HTMLSelectedContentElement &selectedcontent) noexcept;

    /// @see https://html.spec.whatwg.org/#clone-an-option-into-a-selectedcontent
    static void CloneOptionIntoSelectedContent(HTMLOptionElement &option,
                                               HTMLSelectedContentElement &selectedcontent) noexcept;

    /// @see https://html.spec.whatwg.org/#clear-a-selectedcontent
    static void ClearSelectedContent(HTMLSelectedContentElement &selectedcontent) noexcept;

    /// @brief Queues a microtask to update the select's descendant selectedcontent elements.
    static void QueueUpdateDescendantSelectedContents(HTMLSelectElement &select) noexcept;

#pragma endregion

#pragma region HTMLOptionElement

    /// @see https://html.spec.whatwg.org/#update-descendant-selectedcontent-elements-for-an-option
    static void UpdateDescendantSelectedContentsForOption(HTMLOptionElement &option) noexcept;

#pragma endregion
  };
}
