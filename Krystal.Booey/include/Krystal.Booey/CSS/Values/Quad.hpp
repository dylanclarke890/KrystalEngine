#pragma once

#include "Krystal.Booey/CSS/Values/RectBase.hpp"

namespace krys::boo::css
{
  /// @brief Four side values that serialise in the shortest form, e.g. `1px 2px` for `1px 2px 1px 2px`.
  class Quad final : public RectBase
  {
  public:
    explicit Quad(Ref<Value> value) noexcept : RectBase(krys::move(value))
    {
    }

    Quad(Ref<Value> top, Ref<Value> right, Ref<Value> bottom, Ref<Value> left) noexcept
        : RectBase(krys::move(top), krys::move(right), krys::move(bottom), krys::move(left))
    {
    }

    KRYS_NODISCARD CSSOMString CssText(const SerialisationContext &context) const noexcept
    {
      return Serialise(Top().CssText(context), Right().CssText(context), Bottom().CssText(context),
                       Left().CssText(context));
    }

    /// @brief Serialises four side values, omitting the trailing ones that repeat their opposite side.
    KRYS_NODISCARD static CSSOMString Serialise(const CSSOMString &top, const CSSOMString &right,
                                                const CSSOMString &bottom, const CSSOMString &left) noexcept
    {
      if (left != right)
      {
        return top + u8' ' + right + u8' ' + bottom + u8' ' + left;
      }

      if (bottom != top)
      {
        return top + u8' ' + right + u8' ' + bottom;
      }

      if (right != top)
      {
        return top + u8' ' + right;
      }

      return top;
    }
  };
}
