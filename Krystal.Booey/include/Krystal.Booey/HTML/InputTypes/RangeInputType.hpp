/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/InputTypes/InputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/InputTypeNames.hpp"

namespace krys::boo::html
{
  class SliderThumbElement;

  /// @brief The range input type: a slider, with a track and a thumb in its shadow tree.
  class RangeInputType final : public InputType
  {
  private:
    /// @brief The valid values of the datalist's options, sorted; recomputed when the datalist changes.
    List<Decimal> _tickMarkValues;
    bool _tickMarkValuesDirty {true};

  public:
    explicit RangeInputType(HTMLInputElement &element) noexcept : InputType(Type::Range, element)
    {
    }

    KRYS_NODISCARD bool ShouldRespectListAttribute() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Range;
    }

    KRYS_NODISCARD double ValueAsDouble() const noexcept final;
    dom::ExceptionOr<void> SetValueAsDecimal(const Decimal &value,
                                             TextFieldEventBehavior eventBehavior) const noexcept final;
    KRYS_NODISCARD bool TypeMismatchFor(dom::DOMStringView value) const noexcept final;
    KRYS_NODISCARD bool SupportsRequired() const noexcept final;
    KRYS_NODISCARD StepRange CreateStepRange(AnyStepHandling anyStepHandling) const noexcept final;
    ShouldCallBaseEventHandler HandleKeydownEvent(dom::KeyboardEvent &event) noexcept final;
    KRYS_NODISCARD Decimal ParseToNumber(dom::DOMStringView string,
                                         const Decimal &defaultValue) const noexcept final;
    KRYS_NODISCARD dom::DOMString Serialize(const Decimal &value) const noexcept final;
    void AttributeChanged(dom::DOMStringAtom localName) noexcept final;
    void SetValue(dom::DOMStringView value, bool valueChanged, TextFieldEventBehavior eventBehavior,
                  TextControlSetValueSelection selection) noexcept final;
    KRYS_NODISCARD Maybe<dom::DOMString> FallbackValue() const noexcept final;
    KRYS_NODISCARD dom::DOMString SanitizeValue(dom::DOMStringView proposedValue) const noexcept final;

    void HandleMouseDownEvent(dom::MouseEvent &event) noexcept final;
    void DisabledStateChanged() noexcept final;
    void CreateShadowSubtree() noexcept final;
    KRYS_NODISCARD HTMLElement *SliderThumbElement() const noexcept final;
    KRYS_NODISCARD HTMLElement *SliderTrackElement() const noexcept final;
    KRYS_NODISCARD RenderPtr<RenderElement> CreateInputRenderer(RenderStyle &&style) noexcept final;
    void DataListMayHaveChanged() noexcept final;
    KRYS_NODISCARD Maybe<Decimal> FindClosestTickMarkValue(const Decimal &value) noexcept final;

  private:
    void UpdateTickMarkValues() noexcept;
    KRYS_NODISCARD html::SliderThumbElement &TypedSliderThumbElement() const noexcept;
  };
}
