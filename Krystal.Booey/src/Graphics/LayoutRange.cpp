#include "Krystal.Booey/Graphics/LayoutRange.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, const LayoutRange &range) noexcept
  {
    if (ts.HasFormattingFlag(TextStream::Formatting::LayoutUnitsAsIntegers))
    {
      return ts << '[' << range.Min().ToInt() << ',' << range.Max().ToInt() << ']';
    }

    return ts << '[' << range.Min().ToFloat() << ',' << range.Max().ToFloat() << ']';
  }
}
