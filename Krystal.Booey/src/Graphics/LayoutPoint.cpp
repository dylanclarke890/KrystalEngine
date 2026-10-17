#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, const LayoutPoint &p) noexcept
  {
    if (ts.HasFormattingFlag(TextStream::Formatting::LayoutUnitsAsIntegers))
    {
      return ts << '(' << p.X().ToInt() << ',' << p.Y().ToInt() << ')';
    }

    return ts << '(' << p.X().ToFloat() << ',' << p.Y().ToFloat() << ')';
  }
}
