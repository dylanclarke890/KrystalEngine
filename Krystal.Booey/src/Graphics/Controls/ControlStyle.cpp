#include "Krystal.Booey/Graphics/Controls/ControlStyle.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, ControlStyleState state) noexcept
  {
    switch (state)
    {
      case ControlStyleState::Hovered:                  ts << "hovered"; break;
      case ControlStyleState::Pressed:                  ts << "pressed"; break;
      case ControlStyleState::Focused:                  ts << "focused"; break;
      case ControlStyleState::Enabled:                  ts << "enabled"; break;
      case ControlStyleState::Checked:                  ts << "checked"; break;
      case ControlStyleState::Default:                  ts << "default"; break;
      case ControlStyleState::WindowActive:             ts << "window-active"; break;
      case ControlStyleState::Indeterminate:            ts << "indeterminate"; break;
      case ControlStyleState::SpinUp:                   ts << "spin-up"; break;
      case ControlStyleState::Presenting:               ts << "presenting"; break;
      case ControlStyleState::FormSemanticContext:      ts << "form-semantic-context"; break;
      case ControlStyleState::DarkAppearance:           ts << "dark-appearance"; break;
      case ControlStyleState::InlineFlippedWritingMode: ts << "inline-flipped-writing-mode"; break;
      case ControlStyleState::LargeControls:            ts << "large-controls"; break;
      case ControlStyleState::ReadOnly:                 ts << "read-only"; break;
      case ControlStyleState::ListButton:               ts << "list-button"; break;
      case ControlStyleState::ListButtonPressed:        ts << "list-button-pressed"; break;
      case ControlStyleState::VerticalWritingMode:      ts << "vertical-writing-mode"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, const ControlStyle &style) noexcept
  {
    ts.DumpProperty(u8"states", style.States);
    ts.DumpProperty(u8"font-size", style.FontSize);
    ts.DumpProperty(u8"zoom-factor", style.ZoomFactor);
    ts.DumpProperty(u8"accent-color", style.AccentColor);
    return ts;
  }
}
