#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, const LayoutSize &size) noexcept
  {
    return ts << "width=" << size.Width().ToFloat() << " height=" << size.Height().ToFloat();
  }
}
