#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, const LayoutUnit &v) noexcept
  {
    return ts << TextStream::FormatNumberRespectingIntegers(v.ToFloat());
  }
}
