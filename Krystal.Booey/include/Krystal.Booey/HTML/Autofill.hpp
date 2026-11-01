#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::html
{
  class HTMLFormControlElement;

  /// @brief Whether an element's `autocomplete` attribute describes what it expects the user to enter, or
  /// gives the meaning of the value it already holds (on hidden inputs).
  /// @see https://html.spec.whatwg.org/#autofill-expectation-mantle
  enum class AutofillMantle : uint8
  {
    Expectation,
    Anchor
  };

  /// @see https://html.spec.whatwg.org/#non-autofill-credential-type
  enum class NonAutofillCredentialType : bool
  {
    None,
    WebAuthn
  };

  /// @brief What an element's `autocomplete` attribute asks of autofill.
  /// @see https://html.spec.whatwg.org/#autofill-processing-model
  struct AutofillData
  {
    /// @see https://html.spec.whatwg.org/#autofill-field-name
    dom::DOMString FieldName;

    /// @see https://html.spec.whatwg.org/#autofill-hint-set
    List<dom::DOMString> HintSet;

    /// @see https://html.spec.whatwg.org/#autofill-scope
    List<dom::DOMString> Scope;

    /// @see https://html.spec.whatwg.org/#non-autofill-credential-type
    NonAutofillCredentialType CredentialType {NonAutofillCredentialType::None};

    /// @see https://html.spec.whatwg.org/#idl-exposed-autofill-value
    dom::DOMString IDLExposedValue;

    /// @brief The autofill data of an input, select or textarea, from its `autocomplete` attribute.
    KRYS_NODISCARD static AutofillData Create(const HTMLFormControlElement &element) noexcept;
  };
}
