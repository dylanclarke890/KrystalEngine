#include "Krystal.Booey/Graphics/RotationDirection.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, RotationDirection direction) noexcept
  {
    switch (direction)
    {
      case RotationDirection::Counterclockwise: ts << "counterclockwise"; break;
      case RotationDirection::Clockwise:        ts << "clockwise"; break;
    }
    return ts;
  }
}
