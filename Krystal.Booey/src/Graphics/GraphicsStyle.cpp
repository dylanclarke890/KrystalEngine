#include "Krystal.Booey/Graphics/GraphicsStyle.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, const GraphicsDropShadow &dropShadow) noexcept
  {
    ts.DumpProperty(u8"offset", dropShadow.Offset);
    ts.DumpProperty(u8"radius", dropShadow.Radius);
    ts.DumpProperty(u8"color", dropShadow.Color);
    ts.DumpProperty(u8"shadows-use-legacy-radius", dropShadow.RadiusMode == ShadowRadiusMode::Legacy);
    ts.DumpProperty(u8"opacity", dropShadow.Opacity);
    return ts;
  }

  TextStream &operator<<(TextStream &ts, const GraphicsGaussianBlur &gaussianBlur) noexcept
  {
    ts.DumpProperty(u8"radius", gaussianBlur.Radius);
    return ts;
  }

  TextStream &operator<<(TextStream &ts, const GraphicsColorMatrix &colorMatrix) noexcept
  {
    ts.DumpProperty(u8"values", Span<const float> {colorMatrix.Values});
    return ts;
  }

  TextStream &operator<<(TextStream &ts, const GraphicsStyle &style) noexcept
  {
    std::visit([&](const auto &value) { ts << value; }, style);
    return ts;
  }
}
