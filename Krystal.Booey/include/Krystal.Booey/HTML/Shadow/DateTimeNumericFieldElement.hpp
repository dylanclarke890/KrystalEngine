/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 * Copyright (C) 2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/HTML/Shadow/DateTimeFieldElement.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"

namespace krys::boo::html
{
  /// @brief A date or time field holding a number in a range: typing digits sets it.
  class DateTimeNumericFieldElement : public DateTimeFieldElement
  {
  public:
    struct Range
    {
      int Minimum;
      int Maximum;

      KRYS_NODISCARD int ClampValue(int value) const noexcept;
      KRYS_NODISCARD bool IsInRange(int value) const noexcept;
    };

  private:
    const Range _range;
    const dom::DOMString _placeholder;
    int _placeholderValue {0};
    int _value {0};
    bool _hasValue {false};
    dom::DOMString _typeAheadBuffer;
    MonotonicTime::duration _lastDigitCharTime {};

  protected:
    DateTimeNumericFieldElement(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner,
                                const Range &range, int placeholder) noexcept;

    KRYS_NODISCARD int Maximum() const noexcept;

    // DateTimeFieldElement functions:
    KRYS_NODISCARD bool HasValue() const noexcept final;
    void SetEmptyValue(EventBehavior eventBehavior = EventBehavior::DispatchNoEvent) noexcept final;
    void SetValueAsInteger(int value,
                           EventBehavior eventBehavior = EventBehavior::DispatchNoEvent) noexcept final;
    void StepDown() noexcept override;
    void StepUp() noexcept override;

    KRYS_NODISCARD int ValueAsInteger() const noexcept final
    {
      return _hasValue ? _value : -1;
    }

    KRYS_NODISCARD int PlaceholderValueAsInteger() const noexcept final
    {
      return _placeholderValue;
    }

  private:
    // DateTimeFieldElement functions:
    void AdjustMinInlineSize(RenderStyle &style) const noexcept final;
    KRYS_NODISCARD dom::DOMString Value() const noexcept final;
    KRYS_NODISCARD dom::DOMString PlaceholderValue() const noexcept final;
    void HandleKeyboardEvent(dom::KeyboardEvent &event) noexcept final;
    void HandleBlurEvent(dom::Event &event) noexcept final;

    KRYS_NODISCARD dom::DOMString FormatValue(int value) const noexcept;
    void SetValueAsIntegerByStepping(int value) noexcept;
  };
}
