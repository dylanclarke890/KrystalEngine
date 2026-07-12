#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  class Element;
  class HTMLOptionElement;
  class HTMLSelectedContentElement;
  class HTMLSelectElement;

  using HTMLOptionElementList = SmallList<Ref<HTMLOptionElement>>;

  class FormControlAlgorithms
  {
  public:
#pragma region HTMLSelectElement

    /// @see https://html.spec.whatwg.org/#concept-select-size
    KRYS_NODISCARD static size_t DisplaySize(const HTMLSelectElement &element) noexcept;

    /// @see https://html.spec.whatwg.org/#concept-select-option-list
    KRYS_NODISCARD static HTMLOptionElementList ListOfOptions(const HTMLSelectElement &select) noexcept;

    /// @see https://html.spec.whatwg.org/#selectedness-setting-algorithm
    static void SelectednessSettingAlgorithm(HTMLSelectElement &element) noexcept;

#pragma endregion

#pragma region HTMLSelectedContentElement

    /// @see https://html.spec.whatwg.org/#select-enabled-selectedcontent
    KRYS_NODISCARD static RawPtr<HTMLSelectedContentElement>
      GetEnabledSelectedContent(HTMLSelectElement &select) noexcept;

    /// @see https://html.spec.whatwg.org/#clone-an-option-into-a-selectedcontent
    static void CloneOptionIntoSelectedContent(HTMLOptionElement &option,
                                               HTMLSelectedContentElement &selectedcontent) noexcept;
#pragma endregion

#pragma region HTMLOptionElement

    /// @see https://html.spec.whatwg.org/#concept-option-disabled
    KRYS_NODISCARD static bool IsDisabled(const HTMLOptionElement &option) noexcept;

    /// @see https://html.spec.whatwg.org/#update-an-option's-nearest-ancestor-select
    KRYS_NODISCARD static void UpdateNearestAncestorSelect(HTMLOptionElement &option) noexcept;

    /// @see https://html.spec.whatwg.org/#option-element-nearest-ancestor-select
    KRYS_NODISCARD static RawPtr<HTMLSelectElement> NearestAncestorSelect(Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/#maybe-clone-an-option-into-selectedcontent
    static void MaybeCloneOptionIntoSelectedContent(HTMLOptionElement &option) noexcept;

    /// @see https://html.spec.whatwg.org/#maybe-clone-an-option-into-selectedcontent
    static void CloneSelectedOptionIntoButton(HTMLSelectElement &select) noexcept;

    /// @see https://html.spec.whatwg.org/#collect-option-text
    KRYS_NODISCARD static DOMString CollectOptionText(const HTMLOptionElement &option,
                                                      bool includeAltText) noexcept;

#pragma endregion
  };
}