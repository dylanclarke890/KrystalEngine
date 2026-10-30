#include "Krystal.Booey/Graphics/RenderingMode.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, RenderingPurpose purpose) noexcept
  {
    switch (purpose)
    {
      case RenderingPurpose::Unspecified:            ts << "Unspecified"; break;
      case RenderingPurpose::Canvas:                 ts << "Canvas"; break;
      case RenderingPurpose::DOM:                    ts << "DOM"; break;
      case RenderingPurpose::LayerBacking:           ts << "LayerBacking"; break;
      case RenderingPurpose::Snapshot:               ts << "Snapshot"; break;
      case RenderingPurpose::ShareableSnapshot:      ts << "ShareableSnapshot"; break;
      case RenderingPurpose::ShareableLocalSnapshot: ts << "ShareableLocalSnapshot"; break;
      case RenderingPurpose::MediaPainting:          ts << "MediaPainting"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, RenderingMode mode) noexcept
  {
    switch (mode)
    {
      case RenderingMode::Unaccelerated: ts << "Unaccelerated"; break;
      case RenderingMode::Accelerated:   ts << "Accelerated"; break;
      case RenderingMode::DisplayList:   ts << "DisplayList"; break;
    }
    return ts;
  }
}
