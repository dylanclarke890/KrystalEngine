#pragma once

#include "Krystal.Booey/Graphics/SourceImage.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief Whether a pattern repeats along each axis, and the transform from its space to the user space.
  struct PatternParameters
  {
    bool RepeatX {true};
    bool RepeatY {true};
    AffineTransform PatternSpaceTransform;
  };

  /// @brief An image tiled as a brush.
  class Pattern final : public RefCounted<Pattern>
  {
  public:
    using Parameters = PatternParameters;

  private:
    SourceImage _tileImage;
    Parameters _parameters;

  public:
    KRYS_NODISCARD static Ref<Pattern> Create(SourceImage &&tileImage, const Parameters & = {}) noexcept;
    ~Pattern() noexcept;

    KRYS_NODISCARD const SourceImage &TileImage() const noexcept;
    void SetTileImage(SourceImage &&) noexcept;

    KRYS_NODISCARD RefPtr<NativeImage> TileNativeImage() const noexcept;
    KRYS_NODISCARD RefPtr<ImageBuffer> TileImageBuffer() const noexcept;

    KRYS_NODISCARD const Parameters &GetParameters() const noexcept
    {
      return _parameters;
    }

    void SetPatternSpaceTransform(const AffineTransform &) noexcept;

    KRYS_NODISCARD const AffineTransform &PatternSpaceTransform() const noexcept
    {
      return _parameters.PatternSpaceTransform;
    }
    KRYS_NODISCARD bool RepeatX() const noexcept
    {
      return _parameters.RepeatX;
    }
    KRYS_NODISCARD bool RepeatY() const noexcept
    {
      return _parameters.RepeatY;
    }

  private:
    Pattern(SourceImage &&, const Parameters &) noexcept;
  };

  TextStream &operator<<(TextStream &, const Pattern &) noexcept;
}
