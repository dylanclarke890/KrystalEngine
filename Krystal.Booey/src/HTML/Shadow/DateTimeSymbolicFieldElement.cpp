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

#include "Krystal.Booey/HTML/Shadow/DateTimeSymbolicFieldElement.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/KeyboardEvent.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include <algorithm>

namespace krys::boo::html
{
  DateTimeSymbolicFieldElement::DateTimeSymbolicFieldElement(dom::Document &document,
                                                             DateTimeFieldElementFieldOwner &fieldOwner,
                                                             const List<utf8_string> &symbols,
                                                             int placeholderIndex) noexcept
      : DateTimeFieldElement(document, fieldOwner), _symbols(symbols), _placeholderIndex(placeholderIndex),
        _typeAhead(this)
  {
    krys_debug_assert(!_symbols.empty());
  }

  void DateTimeSymbolicFieldElement::AdjustMinInlineSize(RenderStyle &style) const noexcept
  {
    auto &font = style.GetFontCascade();

    float inlineSize = 0;
    for (auto &symbol : _symbols)
    {
      inlineSize = std::max(inlineSize, font.Width(RenderBlock::ConstructTextRun(symbol, style)));
    }

    style.SetLogicalMinWidth(style::MinimumSize::Fixed {inlineSize / style.UsedZoomForLength().Value});
  }

  bool DateTimeSymbolicFieldElement::HasValue() const noexcept
  {
    return _selectedIndex >= 0;
  }

  void DateTimeSymbolicFieldElement::SetEmptyValue(EventBehavior eventBehavior) noexcept
  {
    _selectedIndex = InvalidIndex;
    UpdateVisibleValue(eventBehavior);
  }

  void DateTimeSymbolicFieldElement::SetValueAsInteger(int newSelectedIndex,
                                                       EventBehavior eventBehavior) noexcept
  {
    _selectedIndex = std::max(0, std::min(newSelectedIndex, static_cast<int>(_symbols.size() - 1)));
    UpdateVisibleValue(eventBehavior);
  }

  void DateTimeSymbolicFieldElement::StepDown() noexcept
  {
    int newValue = HasValue() ? _selectedIndex - 1 : static_cast<int>(_symbols.size() - 1);
    if (newValue < 0)
    {
      newValue = static_cast<int>(_symbols.size() - 1);
    }
    SetValueAsInteger(newValue, EventBehavior::DispatchInputAndChangeEvents);
  }

  void DateTimeSymbolicFieldElement::StepUp() noexcept
  {
    int newValue = HasValue() ? _selectedIndex + 1 : 0;
    if (newValue >= static_cast<int>(_symbols.size()))
    {
      newValue = 0;
    }
    SetValueAsInteger(newValue, EventBehavior::DispatchInputAndChangeEvents);
  }

  dom::DOMString DateTimeSymbolicFieldElement::Value() const noexcept
  {
    if (HasValue())
    {
      return _symbols[static_cast<size_t>(_selectedIndex)];
    }
    return {};
  }

  dom::DOMString DateTimeSymbolicFieldElement::PlaceholderValue() const noexcept
  {
    return _symbols[static_cast<size_t>(_placeholderIndex)];
  }

  void DateTimeSymbolicFieldElement::HandleKeyboardEvent(dom::KeyboardEvent &keyboardEvent) noexcept
  {
    if (keyboardEvent.Type() != EventNames::KeyPress)
    {
      return;
    }

    keyboardEvent.SetDefaultHandled();

    constexpr EnumSet<TypeAhead::MatchMode> matchMode {TypeAhead::MatchMode::MatchPrefix,
                                                       TypeAhead::MatchMode::CycleFirstChar,
                                                       TypeAhead::MatchMode::MatchIndex};
    int index = _typeAhead.HandleEvent(keyboardEvent, matchMode);
    if (index < 0)
    {
      // A field has few names: when what was typed matches none of them (`a` then `p` for AM and PM), the key
      // starts again by itself.
      _typeAhead.ResetSession();
      index = _typeAhead.HandleEvent(keyboardEvent, matchMode);
    }
    if (index < 0)
    {
      return;
    }
    SetValueAsInteger(index, EventBehavior::DispatchInputAndChangeEvents);
  }

  int32 DateTimeSymbolicFieldElement::IndexOfSelectedOption() const noexcept
  {
    return _selectedIndex;
  }

  int32 DateTimeSymbolicFieldElement::OptionCount() const noexcept
  {
    return static_cast<int32>(_symbols.size());
  }

  dom::DOMString DateTimeSymbolicFieldElement::OptionAtIndex(int32 index) const noexcept
  {
    return _symbols[static_cast<size_t>(index)];
  }
}
