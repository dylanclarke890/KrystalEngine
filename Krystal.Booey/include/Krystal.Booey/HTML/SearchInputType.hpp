#pragma once

#include "Krystal.Booey/HTML/BaseTextInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  class SearchFieldCancelButtonElement;
  class SearchFieldResultsButtonElement;

  /// @brief The search input type: a text field with a decoration before its text and a cancel button after
  /// it, which Escape also clears.
  // FIXME: Recent searches (the `results` attribute, which isn't standard, and RenderSearchField's popup) and
  // the incremental search event.
  class SearchInputType final : public BaseTextInputType
  {
  private:
    RefPtr<SearchFieldResultsButtonElement> _resultsButton;
    RefPtr<HTMLElement> _cancelButton;

  public:
    explicit SearchInputType(HTMLInputElement &element) noexcept;

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Search;
    }

    ShouldCallBaseEventHandler HandleKeydownEvent(dom::KeyboardEvent &event) noexcept final;

  private:
    KRYS_NODISCARD bool NeedsContainer() const noexcept final;
    void CreateShadowSubtree() noexcept final;
    void RemoveShadowSubtree() noexcept final;
    KRYS_NODISCARD HTMLElement *ResultsButtonElement() const noexcept final;
    KRYS_NODISCARD HTMLElement *CancelButtonElement() const noexcept final;
    void DidSetValueByUserEdit() noexcept final;
    KRYS_NODISCARD bool SizeShouldIncludeDecoration(int32 defaultSize,
                                                    int32 &preferredSize) const noexcept final;
    KRYS_NODISCARD float DecorationWidth(float inputWidth) const noexcept final;
    void SetValue(dom::DOMStringView sanitizedValue, bool valueChanged, TextFieldEventBehavior eventBehavior,
                  TextControlSetValueSelection selection) noexcept final;
  };
}
