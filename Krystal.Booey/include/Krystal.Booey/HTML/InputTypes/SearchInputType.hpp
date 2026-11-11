/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2014-2018 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/InputTypes/BaseTextInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/InputTypeNames.hpp"

namespace krys::boo::html
{
  class SearchFieldCancelButtonElement;
  class SearchFieldResultsButtonElement;

  /// @brief The search input type: a text field with a decoration before its text and a cancel button after
  /// it, which Escape also clears.
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
