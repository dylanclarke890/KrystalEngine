#pragma once

#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief What shows a text field's datalist suggestions.
  enum class DataListSuggestionActivationType : uint8
  {
    /// @brief The focused field was clicked.
    ControlClicked,
    /// @brief The field's list button was clicked.
    IndicatorClicked,
    /// @brief The user changed the field's text.
    TextChanged,
    /// @brief The field's datalist or its suggestions changed.
    DataListMayHaveChanged,
  };

  /// @brief One of a text field's datalist suggestions: the value it gives the field, and its label if that
  /// differs.
  struct DataListSuggestion
  {
    utf8_string Value;
    utf8_string Label;
  };

  /// @brief The list the host shows a text field's datalist suggestions in, below the field. The field keeps
  /// the keyboard: it passes on the keys that move through the list and choose from it.
  class DataListSuggestionPicker : public RefCounted<DataListSuggestionPicker>
  {
  public:
    virtual ~DataListSuggestionPicker() noexcept = default;

    /// @brief The field's going away: the picker no longer tells it anything.
    virtual void Detach() noexcept
    {
    }

    /// @brief Hides the list; it tells its client it closed.
    virtual void Close() noexcept
    {
    }

    /// @brief A key the field passes on, by its key identifier: "Up", "Down", "Enter" or "U+001B" (Escape).
    virtual void HandleKeydownWithIdentifier(utf8_stringview identifier) noexcept
    {
      (void)identifier;
    }

    /// @brief Shows the client's current suggestions, or hides the list if there are none.
    virtual void DisplayWithActivationType(DataListSuggestionActivationType type) noexcept
    {
      (void)type;
    }
  };
}
