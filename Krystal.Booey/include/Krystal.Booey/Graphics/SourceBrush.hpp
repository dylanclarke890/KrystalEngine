#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/SourceBrushLogicalGradient.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief What a fill or stroke paints with: a colour or a gradient.
  /// FIXME: Patterns (`Pattern`, `setPattern`) come with images.
  class SourceBrush
  {
  public:
    using OptionalPatternGradient = Variant<std::monostate, SourceBrushLogicalGradient>;

  private:
    boo::Color _color {boo::Color::black};
    OptionalPatternGradient _patternGradient;

  public:
    SourceBrush() noexcept = default;
    SourceBrush(const boo::Color &color) noexcept : _color(color)
    {
    }

    // Color should be stored in the variant, in place of std::monostate. Currently a lot of code
    // queries the color unconditionally, color is accessible unconditionally, returning incorrect
    // but defined data.
    KRYS_NODISCARD const boo::Color &GetColor() const noexcept
    {
      return _color;
    }
    void SetColor(const boo::Color color) noexcept
    {
      _color = color;
    }

    // Packed color accessor takes into account the discrimination between color, gradient, pattern.
    KRYS_NODISCARD Maybe<PackedColour::RGBA> PackedColor() const noexcept;

    KRYS_NODISCARD boo::Gradient *GetGradient() const noexcept;
    KRYS_NODISCARD const AffineTransform &GradientSpaceTransform() const noexcept;

    void SetGradient(Ref<boo::Gradient> &&, const AffineTransform &spaceTransform = {}) noexcept;

    KRYS_NODISCARD bool IsVisible() const noexcept
    {
      return HasPatternOrGradient() || _color.IsVisible();
    }
    KRYS_NODISCARD bool HasPatternOrGradient() const noexcept
    {
      return !std::holds_alternative<std::monostate>(_patternGradient);
    }

    KRYS_NODISCARD friend bool operator==(const SourceBrush &a, const SourceBrush &b) noexcept
    {
      return a._color == b._color && a._patternGradient == b._patternGradient;
    }
  };

  inline Maybe<PackedColour::RGBA> SourceBrush::PackedColor() const noexcept
  {
    if (HasPatternOrGradient())
    {
      return std::nullopt;
    }
    return _color.TryGetAsPackedInline();
  }

  TextStream &operator<<(TextStream &ts, const SourceBrush &brush) noexcept;
}
