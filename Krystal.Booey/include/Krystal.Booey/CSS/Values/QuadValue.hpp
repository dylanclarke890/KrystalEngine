#pragma once

#include "Krystal.Booey/CSS/Values/Quad.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A value with four sides, e.g. `margin: 1px 2px`.
  class QuadValue final : public Value
  {
  private:
    bool _coalesceIdenticalValues {true};
    Quad _quad;

    explicit QuadValue(Quad quad) noexcept;

  public:
    KRYS_NODISCARD static Ref<QuadValue> Create(Quad quad) noexcept;

    /// @brief All four sides are `value`.
    KRYS_NODISCARD static Ref<QuadValue> Create(Ref<Value> value) noexcept
    {
      return Create(Quad {value, value, value, value});
    }

    /// @brief `first` for top and bottom, `second` for right and left.
    KRYS_NODISCARD static Ref<QuadValue> Create(Ref<Value> first, Ref<Value> second) noexcept
    {
      return Create(Quad {first, second, first, second});
    }

    /// @brief `first` for top, `second` for right and left, `third` for bottom.
    KRYS_NODISCARD static Ref<QuadValue> Create(Ref<Value> first, Ref<Value> second,
                                                Ref<Value> third) noexcept
    {
      return Create(Quad {first, second, third, second});
    }

    /// @brief Top, right, bottom and left.
    KRYS_NODISCARD static Ref<QuadValue> Create(Ref<Value> first, Ref<Value> second, Ref<Value> third,
                                                Ref<Value> fourth) noexcept
    {
      return Create(Quad {krys::move(first), krys::move(second), krys::move(third), krys::move(fourth)});
    }

    KRYS_NODISCARD const Quad &GetQuad() const noexcept
    {
      return _quad;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const QuadValue &other) const noexcept;

    /// @brief Whether the value coalesces identical values and all four sides are equal.
    KRYS_NODISCARD bool CanBeCoalesced() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::QuadValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsQuadValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

namespace krys::boo::css
{
  inline const Quad &Value::GetQuad() const noexcept
  {
    return Downcast<QuadValue>(*this).GetQuad();
  }
}
