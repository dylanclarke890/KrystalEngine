#include "Krystal.Booey/Core/StyleAppearance.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, StyleAppearance appearance) noexcept
  {
    switch (appearance)
    {
      case StyleAppearance::None:                         ts << "none"; break;
      case StyleAppearance::Auto:                         ts << "auto"; break;
      case StyleAppearance::Base:                         ts << "base"; break;
      case StyleAppearance::Checkbox:                     ts << "checkbox"; break;
      case StyleAppearance::Radio:                        ts << "radio"; break;
      case StyleAppearance::PushButton:                   ts << "push-button"; break;
      case StyleAppearance::SquareButton:                 ts << "square-button"; break;
      case StyleAppearance::Button:                       ts << "button"; break;
      case StyleAppearance::DefaultButton:                ts << "default-button"; break;
      case StyleAppearance::Listbox:                      ts << "listbox"; break;
      case StyleAppearance::Menulist:                     ts << "menulist"; break;
      case StyleAppearance::MenulistButton:               ts << "menulist-button"; break;
      case StyleAppearance::Meter:                        ts << "meter"; break;
      case StyleAppearance::ProgressBar:                  ts << "progress-bar"; break;
      case StyleAppearance::SliderHorizontal:             ts << "slider-horizontal"; break;
      case StyleAppearance::SliderVertical:               ts << "slider-vertical"; break;
      case StyleAppearance::SearchField:                  ts << "searchfield"; break;
      case StyleAppearance::TextArea:                     ts << "textarea"; break;
      case StyleAppearance::TextField:                    ts << "textfield"; break;
      case StyleAppearance::ColorWell:                    ts << "color-well"; break;
      case StyleAppearance::ColorWellSwatch:              ts << "color-well-swatch"; break;
      case StyleAppearance::ColorWellSwatchOverlay:       ts << "color-well-swatch-overlay"; break;
      case StyleAppearance::ColorWellSwatchWrapper:       ts << "color-well-swatch-wrapper"; break;
      case StyleAppearance::InnerSpinButton:              ts << "inner-spin-button"; break;
      case StyleAppearance::ListButton:                   ts << "list-button"; break;
      case StyleAppearance::SearchFieldDecoration:        ts << "searchfield-decoration"; break;
      case StyleAppearance::SearchFieldResultsDecoration: ts << "searchfield-results-decoration"; break;
      case StyleAppearance::SearchFieldResultsButton:     ts << "searchfield-results-button"; break;
      case StyleAppearance::SearchFieldCancelButton:      ts << "searchfield-cancel-button"; break;
      case StyleAppearance::SliderThumbHorizontal:        ts << "sliderthumb-horizontal"; break;
      case StyleAppearance::SliderThumbVertical:          ts << "sliderthumb-vertical"; break;
      case StyleAppearance::Switch:                       ts << "switch"; break;
      case StyleAppearance::SwitchThumb:                  ts << "switch-thumb"; break;
      case StyleAppearance::SwitchTrack:                  ts << "switch-track"; break;
    }
    return ts;
  }
}
