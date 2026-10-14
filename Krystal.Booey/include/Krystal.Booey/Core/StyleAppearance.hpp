#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  // Must follow css-value-keywords.in order
  enum class StyleAppearance : uint8
  {
    None,
    Auto,
    Base,
    Checkbox,
    Radio,
    PushButton,
    SquareButton,
    Button,
    DefaultButton,
    Listbox,
    Menulist,
    MenulistButton,
    Meter,
    ProgressBar,
    SliderHorizontal,
    SliderVertical,
    SearchField,
    TextArea,
    TextField,
    // Internal-only Values
    ColorWell,
    ColorWellSwatch,
    ColorWellSwatchOverlay,
    ColorWellSwatchWrapper,
    InnerSpinButton,
    ListButton,
    SearchFieldDecoration,
    SearchFieldResultsDecoration,
    SearchFieldResultsButton,
    SearchFieldCancelButton,
    SliderThumbHorizontal,
    SliderThumbVertical,
    Switch,
    SwitchThumb,
    SwitchTrack
  };

  constexpr int AppearanceBitWidth = 7;
}
