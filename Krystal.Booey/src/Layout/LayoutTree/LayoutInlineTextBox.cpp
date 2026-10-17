#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"

namespace krys::boo::layout
{
  InlineTextBox::InlineTextBox(utf8_string content, bool isCombined,
                               EnumSet<ContentCharacteristic> contentCharacteristicSet, RenderStyle &&style,
                               UniquePtr<RenderStyle> &&firstLineStyle) noexcept
      : Box({NodeType::Text, AnonymousBox::Yes}, krys::move(style), krys::move(firstLineStyle),
            Box::InlineTextBoxFlag),
        _content(krys::move(content)), _isCombined(isCombined),
        _contentCharacteristicSet(contentCharacteristicSet)
  {
  }
}
