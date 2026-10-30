#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Colour/AlphaPremultiplication.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"
#include "Krystal.Core/Maths/Vector.hpp"
#include "Krystal.Gfx/Handles.hpp"
#include "Krystal.Gfx/Render2D/Gradient2D.hpp"
#include "Krystal.Gfx/Render2D/Transform2D.hpp"
#include "Krystal.Gfx/Sampler.hpp"

namespace krys::gfx
{
  /// @brief An image repeated over what it paints: the whole texture is one tile.
  struct ImagePattern2D
  {
    TextureHandle Texture {};
    /// @brief Maps pattern space into the draw's coordinates. In pattern space, a tile is `TileSize` and
    /// tiles repeat every `TileSize` + `Spacing` from the origin, in both directions.
    Transform2D PatternSpace {};
    Vec2f TileSize {};
    /// @brief The transparent gap after each tile.
    Vec2f Spacing {};
    SamplerFilter Filter {SamplerFilter::Linear};
    /// @brief Whether the texture's colours are already multiplied by their alpha.
    AlphaPremultiplication Alpha {AlphaPremultiplication::Premultiplied};
    /// @brief Multiplies the pattern's alpha, from 0 to 1.
    float Opacity {1.f};
    /// @brief Whether filtering wraps across the tile's edges along each axis; otherwise it clamps to them,
    /// for a tile that fills what it paints without repeating.
    bool RepeatX {true};
    bool RepeatY {true};
  };

  /// @brief What a fill or stroke paints with: a colour, a gradient or a repeated image.
  /// @attention A gradient or pattern paint refers to its gradient or pattern rather than copying it, so it
  /// must outlive the paint; a draw is done with it once the call returns.
  class Paint2D
  {
    SRGBA<uint8> _colour {};
    const Gradient2D *_gradient {nullptr};
    Transform2D _gradientSpace {};
    const ImagePattern2D *_pattern {nullptr};

  public:
    /// @brief A solid colour, sRGB with straight alpha.
    constexpr Paint2D(SRGBA<uint8> colour) noexcept : _colour(colour)
    {
    }

    /// @brief A gradient, its coordinates mapped into the draw's by `gradientSpace`.
    constexpr Paint2D(const Gradient2D &gradient, const Transform2D &gradientSpace = {}) noexcept
        : _gradient(&gradient), _gradientSpace(gradientSpace)
    {
    }

    /// @brief An image repeated as `pattern` lays it out.
    constexpr Paint2D(const ImagePattern2D &pattern) noexcept : _pattern(&pattern)
    {
    }

    KRYS_NODISCARD constexpr SRGBA<uint8> GetColour() const noexcept
    {
      return _colour;
    }

    /// @brief The gradient, or null for a solid colour.
    KRYS_NODISCARD constexpr const Gradient2D *GetGradient() const noexcept
    {
      return _gradient;
    }

    /// @brief The repeated image, or null for a colour or gradient.
    KRYS_NODISCARD constexpr const ImagePattern2D *GetPattern() const noexcept
    {
      return _pattern;
    }

    /// @brief Maps the gradient's coordinates into the coordinates of the draw it paints.
    KRYS_NODISCARD constexpr const Transform2D &GetGradientSpace() const noexcept
    {
      return _gradientSpace;
    }
  };
}
