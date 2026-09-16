#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Position.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  class PositionValue final : public Value
  {
  private:
    Position _position;

    PositionValue(Position &&);

  public:
    KRYS_NODISCARD static Ref<PositionValue> Create(Position &&position) noexcept;

    KRYS_NODISCARD const Position &Position() const noexcept
    {
      return _position;
    }

    KRYS_NODISCARD bool Equals(const PositionValue &other) const noexcept;
  };

  class PositionXValue final : public Value
  {
  private:
    PositionX _position;

    PositionXValue(PositionX &&);

  public:
    KRYS_NODISCARD static Ref<PositionXValue> Create(PositionX &&position) noexcept;

    KRYS_NODISCARD const PositionX &Position() const noexcept
    {
      return _position;
    }

    KRYS_NODISCARD bool Equals(const PositionXValue &other) const noexcept;
  };

  class PositionYValue final : public Value
  {
  private:
    PositionY _position;

    PositionYValue(PositionY &&);

  public:
    KRYS_NODISCARD static Ref<PositionYValue> Create(PositionY &&position) noexcept;

    KRYS_NODISCARD const PositionY &Position() const noexcept
    {
      return _position;
    }

    KRYS_NODISCARD bool Equals(const PositionYValue &other) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::PositionValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsPositionValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::PositionXValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsPositionXValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::PositionYValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsPositionYValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()