#pragma once

#include "Krystal.Booey/Graphics/Geometry/IntRect.hpp"
#include "Krystal.Booey/Platform/Choosers/DataListSuggestionPicker.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  /// @brief Who a datalist suggestion picker shows the suggestions of: a text field. It detaches its picker
  /// before it goes away.
  class DataListSuggestionsClient
  {
  public:
    virtual ~DataListSuggestionsClient() noexcept = default;

    /// @brief Where the field is, in the root view's coordinates, for the list to go below it.
    KRYS_NODISCARD virtual IntRect ElementRectInRootViewCoordinates() const noexcept = 0;
    /// @brief The suggestions for the field's current text: those starting with it, then those containing
    /// it.
    KRYS_NODISCARD virtual List<DataListSuggestion> Suggestions() noexcept = 0;
    /// @brief Whether the field lays out right to left, so the list is mirrored.
    KRYS_NODISCARD virtual bool IsElementRTL() const noexcept = 0;
    virtual void DidSelectDataListOption(utf8_stringview value) noexcept = 0;
    virtual void DidCloseSuggestions() noexcept = 0;
  };
}
