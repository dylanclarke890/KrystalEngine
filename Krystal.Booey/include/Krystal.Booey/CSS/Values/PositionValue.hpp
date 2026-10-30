#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Position.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `<position>` value, e.g. `background-position: left 10px top`.
  class PositionValue final : public Value
  {
  private:
    Position _position;

    explicit PositionValue(Position &&position) noexcept;

  public:
    KRYS_NODISCARD static Ref<PositionValue> Create(Position position) noexcept;

    KRYS_NODISCARD const Position &GetPosition() const noexcept
    {
      return _position;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const PositionValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
  };

  /// @brief The horizontal half of a `<position>`, e.g. `background-position-x: right 10px`.
  class PositionXValue final : public Value
  {
  private:
    PositionX _position;

    explicit PositionXValue(PositionX &&position) noexcept;

  public:
    KRYS_NODISCARD static Ref<PositionXValue> Create(PositionX position) noexcept;

    KRYS_NODISCARD const PositionX &GetPosition() const noexcept
    {
      return _position;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const PositionXValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
  };

  /// @brief The vertical half of a `<position>`, e.g. `background-position-y: center`.
  class PositionYValue final : public Value
  {
  private:
    PositionY _position;

    explicit PositionYValue(PositionY &&position) noexcept;

  public:
    KRYS_NODISCARD static Ref<PositionYValue> Create(PositionY position) noexcept;

    KRYS_NODISCARD const PositionY &GetPosition() const noexcept
    {
      return _position;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const PositionYValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
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
