#include "Krystal.Booey/Graphics/Pattern.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<Pattern> Pattern::Create(SourceImage &&tileImage, const Parameters &parameters) noexcept
  {
    return AdoptRef(*new Pattern(krys::move(tileImage), parameters));
  }

  Pattern::Pattern(SourceImage &&tileImage, const Parameters &parameters) noexcept
      : _tileImage(krys::move(tileImage)), _parameters(parameters)
  {
  }

  Pattern::~Pattern() noexcept = default;

  void Pattern::SetPatternSpaceTransform(const AffineTransform &patternSpaceTransform) noexcept
  {
    _parameters.PatternSpaceTransform = patternSpaceTransform;
  }

  const SourceImage &Pattern::TileImage() const noexcept
  {
    return _tileImage;
  }

  RefPtr<NativeImage> Pattern::TileNativeImage() const noexcept
  {
    return ShareRefPtr(_tileImage.GetNativeImage());
  }

  RefPtr<ImageBuffer> Pattern::TileImageBuffer() const noexcept
  {
    return ShareRefPtr(_tileImage.GetImageBuffer());
  }

  void Pattern::SetTileImage(SourceImage &&tileImage) noexcept
  {
    _tileImage = krys::move(tileImage);
  }

  TextStream &operator<<(TextStream &ts, const Pattern &pattern) noexcept
  {
    ts.DumpProperty(u8"tile-size", pattern.TileImage().Size());
    ts.DumpProperty(u8"pattern-space-transform", pattern.PatternSpaceTransform());
    return ts;
  }
}
