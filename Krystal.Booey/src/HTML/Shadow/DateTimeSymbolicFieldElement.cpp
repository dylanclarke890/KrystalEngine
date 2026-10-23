#include "Krystal.Booey/HTML/Shadow/DateTimeSymbolicFieldElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
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

    int index = _typeAhead.HandleEvent(keyboardEvent, {TypeAhead::MatchMode::MatchPrefix,
                                                       TypeAhead::MatchMode::CycleFirstChar,
                                                       TypeAhead::MatchMode::MatchIndex});
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
