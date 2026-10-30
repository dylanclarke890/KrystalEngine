#pragma once

#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief What a date chooser shows: the input's type, value, range and step, and where it is.
  struct DateTimeChooserParameters
  {
    utf8_string Type;
    IntRect AnchorRectInRootView;
    /// @brief The locale the chooser should be localized for. It might be an invalid name, because it comes
    /// from HTML `lang` attributes.
    utf8_string Locale;
    utf8_string CurrentValue;
    List<utf8_string> SuggestionValues;
    List<utf8_string> LocalizedSuggestionValues;
    List<utf8_string> SuggestionLabels;
    double Minimum {0};
    double Maximum {0};
    double Step {0};
    double StepBase {0};
    bool Required {false};
    bool IsAnchorElementRTL {false};
    bool UseDarkAppearance {false};
    bool HasSecondField {false};
    bool HasMillisecondField {false};
    bool WasActivatedByKeyboard {false};
  };
}
