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

#include "Krystal.Booey/HTML/Forms/TypeAhead.hpp"
#include "Krystal.Booey/HTML/Shadow/DateTimeFieldElement.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::html
{
  /// @brief A date or time field holding one of a list of names (a month's, AM or PM): typing the start of
  /// one chooses it.
  class DateTimeSymbolicFieldElement : public DateTimeFieldElement, public TypeAheadDataSource
  {
  private:
    static constexpr int InvalidIndex = -1;

    const List<utf8_string> _symbols;
    int _selectedIndex {InvalidIndex};
    int _placeholderIndex {InvalidIndex};

    TypeAhead _typeAhead;

  protected:
    DateTimeSymbolicFieldElement(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner,
                                 const List<utf8_string> &symbols, int placeholderIndex) noexcept;

    KRYS_NODISCARD size_t SymbolsSize() const noexcept
    {
      return _symbols.size();
    }

    KRYS_NODISCARD bool HasValue() const noexcept final;
    void SetEmptyValue(EventBehavior eventBehavior = EventBehavior::DispatchNoEvent) noexcept override;
    void SetValueAsInteger(int newSelectedIndex,
                           EventBehavior eventBehavior = EventBehavior::DispatchNoEvent) noexcept override;

    KRYS_NODISCARD int ValueAsInteger() const noexcept final
    {
      return _selectedIndex;
    }

    KRYS_NODISCARD int PlaceholderValueAsInteger() const noexcept final
    {
      return _placeholderIndex;
    }

  private:
    // DateTimeFieldElement functions:
    void AdjustMinInlineSize(RenderStyle &style) const noexcept final;
    void StepDown() noexcept final;
    void StepUp() noexcept final;
    KRYS_NODISCARD dom::DOMString Value() const noexcept final;
    KRYS_NODISCARD dom::DOMString PlaceholderValue() const noexcept final;
    void HandleKeyboardEvent(dom::KeyboardEvent &event) noexcept final;

    // TypeAheadDataSource functions:
    KRYS_NODISCARD int32 IndexOfSelectedOption() const noexcept final;
    KRYS_NODISCARD int32 OptionCount() const noexcept final;
    KRYS_NODISCARD dom::DOMString OptionAtIndex(int32 index) const noexcept final;
  };
}
