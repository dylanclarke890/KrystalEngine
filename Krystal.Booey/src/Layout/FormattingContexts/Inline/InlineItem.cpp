#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineTextItem.hpp"

namespace krys::boo::layout
{
  namespace
  {
    struct SameSizeAsInlineItem
    {
      void *LayoutBox;
      float Width;
      uint32 Length;
      uint32 Start;
      uint8 BidiLevel;
      uint8 Type : 3;
      uint8 TextItemType : 2;
      bool WidthBool : 1;
      bool SoftHyphenBool : 1;
      bool IsWordSeparator : 1;
    };
  }

  static_assert(sizeof(InlineItem) == sizeof(SameSizeAsInlineItem));
}
