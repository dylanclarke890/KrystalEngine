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

  boo::Pattern *SourceBrush::GetPattern() const noexcept
  {
    if (auto *pattern = std::get_if<Ref<boo::Pattern>>(&_patternGradient))
    {
      return pattern->get();
    }
    return nullptr;
  }

  void SourceBrush::SetGradient(Ref<boo::Gradient> &&gradient, const AffineTransform &spaceTransform) noexcept
  {
    _patternGradient = SourceBrushLogicalGradient {krys::move(gradient), spaceTransform};
  }

  void SourceBrush::SetPattern(Ref<boo::Pattern> &&pattern) noexcept
  {
    _patternGradient.emplace<Ref<boo::Pattern>>(krys::move(pattern));
  }

  TextStream &operator<<(TextStream &ts, const SourceBrush &brush) noexcept
  {
    ts.DumpProperty(u8"color", brush.GetColor());
    if (auto *gradient = brush.GetGradient())
    {
      ts.DumpProperty(u8"gradient", *gradient);
      ts.DumpProperty(u8"gradient-space-transform", brush.GradientSpaceTransform());
    }
    if (auto *pattern = brush.GetPattern())
    {
      ts.DumpProperty(u8"pattern", *pattern);
    }
    return ts;
  }
}
