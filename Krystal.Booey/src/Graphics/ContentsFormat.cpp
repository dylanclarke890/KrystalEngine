#include "Krystal.Booey/Graphics/ContentsFormat.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, ContentsFormat contentsFormat) noexcept
  {
    switch (contentsFormat)
    {
      case ContentsFormat::RGBA8: ts << "RGBA8"; break;
    }
    return ts;
  }
}
