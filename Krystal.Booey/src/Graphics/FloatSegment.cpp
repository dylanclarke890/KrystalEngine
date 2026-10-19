#include "Krystal.Booey/Graphics/FloatSegment.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, FloatSegment floatSegment) noexcept
  {
    TextStream::GroupScope scope(ts);
    ts.DumpProperty(u8"begin", floatSegment.Begin);
    ts.DumpProperty(u8"end", floatSegment.End);
    return ts;
  }
}
