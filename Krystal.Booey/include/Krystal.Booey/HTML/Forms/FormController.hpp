#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::html
{
  class FormListedElement;
  class HTMLFormElement;
  class ValidatedFormListedElement;

  /// @brief A form control's state, as it saves it: its value, its checkedness, its selected options.
  using FormControlState = List<dom::DOMString>;

  /// @brief Saves the state of a document's form controls (to its history item, as the document is left) and
  /// restores it to the controls of the document loaded for the item again, as they're parsed: each control
  /// is matched by its form, its name and its type.
  class FormController final
  {
    KRYS_NON_COPY_MOVABLE(FormController);

  private:
    class FormKeyGenerator;
    class SavedFormState;
    using SavedFormStateMap = HashMap<dom::DOMString, SavedFormState>;

    UniquePtr<SavedFormStateMap> _savedFormStateMap;
    UniquePtr<FormKeyGenerator> _formKeyGenerator;

  public:
    FormController() noexcept;
    ~FormController() noexcept;

    /// @brief The state of @p document's form controls, serialized.
    KRYS_NODISCARD List<dom::DOMString> FormElementsState(const dom::Document &document) const noexcept;

    /// @brief The state the controls parsed from now on are given, as @p stateVector serializes it.
    void SetStateForNewFormElements(const List<dom::DOMString> &stateVector) noexcept;

    void WillDeleteForm(HTMLFormElement &form) noexcept;

    /// @brief Gives @p control its saved state, if it has some.
    void RestoreControlStateFor(ValidatedFormListedElement &control) noexcept;

    /// @brief Gives the controls of @p form their saved state, once the form is parsed.
    void RestoreControlStateIn(HTMLFormElement &form) noexcept;

    KRYS_NODISCARD bool HasFormStateToRestore() const noexcept;

    /// @brief The form @p control's state is saved and restored with: none for a control with a `form`
    /// attribute, whose form may not be known while it's parsed.
    KRYS_NODISCARD static HTMLFormElement *OwnerForm(const FormListedElement &control) noexcept;

  private:
    KRYS_NODISCARD FormControlState
      TakeStateForFormElement(const ValidatedFormListedElement &control) noexcept;
    KRYS_NODISCARD static SavedFormStateMap
      ParseStateVector(const List<dom::DOMString> &stateVector) noexcept;
  };
}
