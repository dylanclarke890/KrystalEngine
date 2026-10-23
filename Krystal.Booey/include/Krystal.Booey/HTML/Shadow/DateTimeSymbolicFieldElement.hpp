#pragma once

#include "Krystal.Booey/HTML/Shadow/DateTimeFieldElement.hpp"
#include "Krystal.Booey/HTML/TypeAhead.hpp"
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
