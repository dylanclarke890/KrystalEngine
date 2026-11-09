/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/HTML/InputTypes/BaseClickableWithKeyInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/InputTypeNames.hpp"
#include "Krystal.Booey/Platform/Choosers/ColorChooser.hpp"
#include "Krystal.Booey/Platform/Choosers/ColorChooserClient.hpp"

namespace krys::boo::html
{
  /// @brief The color input type: a swatch of its colour (the `::-webkit-color-swatch` part), which opens the
  /// host's colour picker when activated.
  class ColorInputType final : public BaseClickableWithKeyInputType, private ColorChooserClient
  {
  private:
    RefPtr<ColorChooser> _chooser;

  public:
    explicit ColorInputType(HTMLInputElement &element) noexcept
        : BaseClickableWithKeyInputType(Type::Color, element)
    {
    }

    ~ColorInputType() noexcept override;
    KRYS_NODISCARD bool ShouldRespectListAttribute() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Color;
    }

    KRYS_NODISCARD bool SupportsRequired() const noexcept final;
    KRYS_NODISCARD Maybe<dom::DOMString> FallbackValue() const noexcept final;
    KRYS_NODISCARD dom::DOMString SanitizeValue(dom::DOMStringView proposedValue) const noexcept final;

    /// @brief The value as a colour.
    KRYS_NODISCARD Color ValueAsColor() const noexcept;

    void Detach() noexcept final;

    // ColorChooserClient functions:
    KRYS_NODISCARD bool SupportsAlpha() const noexcept final;
    KRYS_NODISCARD bool IsElementRTL() const noexcept final;
    KRYS_NODISCARD List<Color> SuggestedColors() const noexcept final;

  private:
    void DidChooseColor(const Color &color) noexcept final;
    void DidEndChooser() noexcept final;
    KRYS_NODISCARD IntRect ElementRectRelativeToRootView() const noexcept final;

    // InputType functions:
    KRYS_NODISCARD bool IsPresentingAttachedView() const noexcept final;
    void CreateShadowSubtree() noexcept final;
    void SetValue(dom::DOMStringView value, bool valueChanged, TextFieldEventBehavior eventBehavior,
                  TextControlSetValueSelection selection) noexcept final;
    void AttributeChanged(dom::DOMStringAtom name) noexcept final;
    void HandleActivationBehavior(dom::Event &event) noexcept final;
    void ShowPicker() noexcept final;
    void ElementDidBlur() noexcept final;

    void EndColorChooser() noexcept;
    void UpdateColorSwatch() noexcept;
    KRYS_NODISCARD HTMLElement *ShadowColorSwatch() const noexcept;
  };
}
