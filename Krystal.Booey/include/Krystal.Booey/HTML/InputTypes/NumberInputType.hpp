/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2018 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/HTML/InputTypes/InputTypeNames.hpp"
#include "Krystal.Booey/HTML/InputTypes/TextFieldInputType.hpp"

namespace krys::boo::html
{
  /// @brief The number input type.
  // FIXME: The spin button (with the theme, G14d), and localised numbers (the element's locale).
  class NumberInputType final : public TextFieldInputType
  {
  public:
    explicit NumberInputType(HTMLInputElement &element) noexcept : TextFieldInputType(Type::Number, element)
    {
    }

    KRYS_NODISCARD DOMStringView FormControlType() const noexcept final
    {
      return input_type_names::Number;
    }

    KRYS_NODISCARD double ValueAsDouble() const noexcept final;

    ExceptionOr<void> SetValueAsDouble(double value,
                                       TextFieldEventBehavior eventBehavior) const noexcept final;

    ExceptionOr<void> SetValueAsDecimal(const Decimal &value,
                                        TextFieldEventBehavior eventBehavior) const noexcept final;

    KRYS_NODISCARD bool TypeMismatchFor(DOMStringView value) const noexcept final;

    KRYS_NODISCARD bool TypeMismatch() const noexcept final;

    KRYS_NODISCARD StepRange CreateStepRange(AnyStepHandling anyStepHandling) const noexcept final;

    KRYS_NODISCARD Decimal ParseToNumber(DOMStringView string,
                                         const Decimal &defaultValue) const noexcept final;

    KRYS_NODISCARD DOMString Serialize(const Decimal &value) const noexcept final;

    KRYS_NODISCARD DOMString SanitizeValue(DOMStringView proposedValue) const noexcept final;

    KRYS_NODISCARD bool HasBadInput() const noexcept final;

    KRYS_NODISCARD DOMString BadInputText() const noexcept final;

    KRYS_NODISCARD bool SupportsPlaceholder() const noexcept final;

  private:
    void SetValue(DOMStringView sanitizedValue, bool valueChanged, TextFieldEventBehavior eventBehavior,
                  TextControlSetValueSelection selection) noexcept final;

    KRYS_NODISCARD bool SizeShouldIncludeDecoration(int32 defaultSize,
                                                    int32 &preferredSize) const noexcept final;

    KRYS_NODISCARD float DecorationWidth(float inputWidth) const noexcept final;

    ShouldCallBaseEventHandler HandleKeydownEvent(KeyboardEvent &event) noexcept final;

    void HandleBeforeTextInsertedEvent(BeforeTextInsertedEvent &event) noexcept final;

    /// @brief @p proposedValue as the element's locale writes numbers (its digits and decimal separator), or
    /// as it is in scientific notation.
    KRYS_NODISCARD DOMString LocalizeValue(DOMStringView proposedValue) const noexcept final;

    KRYS_NODISCARD DOMString VisibleValue() const noexcept final;

    /// @brief @p visibleValue, written in the element's locale, as an HTML floating-point number.
    KRYS_NODISCARD DOMString ConvertFromVisibleValue(DOMStringView visibleValue) const noexcept final;

    void AttributeChanged(const DOMStringAtom &name) noexcept final;
  };
}
