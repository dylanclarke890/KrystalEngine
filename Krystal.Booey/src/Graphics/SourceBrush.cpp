#include "Krystal.Booey/Graphics/SourceBrush.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  const AffineTransform &SourceBrush::GradientSpaceTransform() const noexcept
  {
    if (auto *logicalGradient = std::get_if<SourceBrushLogicalGradient>(&_patternGradient))
    {
      return logicalGradient->SpaceTransform;
    }
    return AffineTransform::Identity;
  }

  boo::Gradient *SourceBrush::GetGradient() const noexcept
  {
    if (auto *logicalGradient = std::get_if<SourceBrushLogicalGradient>(&_patternGradient))
    {
      return logicalGradient->Gradient.get();
    }
    return nullptr;
  }

  void SourceBrush::SetGradient(Ref<boo::Gradient> &&gradient, const AffineTransform &spaceTransform) noexcept
  {
    _patternGradient = SourceBrushLogicalGradient {krys::move(gradient), spaceTransform};
  }

  TextStream &operator<<(TextStream &ts, const SourceBrush &brush) noexcept
  {
    ts.DumpProperty(u8"color", brush.GetColor());
    if (auto *gradient = brush.GetGradient())
    {
      ts.DumpProperty(u8"gradient", *gradient);
      ts.DumpProperty(u8"gradient-space-transform", brush.GradientSpaceTransform());
    }
    return ts;
  }
}
