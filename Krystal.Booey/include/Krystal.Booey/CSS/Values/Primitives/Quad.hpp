#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/RectBase.hpp"

namespace krys::boo::css
{
  class Quad final : public RectBase
  {
  public:
    Quad(Ref<Value> value) : RectBase(krys::move(value))
    {
    }

    Quad(Ref<Value> top, Ref<Value> right, Ref<Value> bottom, Ref<Value> left)
        : RectBase(krys::move(top), krys::move(right), krys::move(bottom), krys::move(left))
    {
    }

    CSSOMString CssText(const SerialisationContext &context) const
    {
      return Serialise(Top().CssText(context), Right().CssText(context), Bottom().CssText(context),
                       Left().CssText(context));
    }

    static CSSOMString Serialise(const CSSOMString &top, const CSSOMString &right, const CSSOMString &bottom,
                                 const CSSOMString &left) noexcept
    {
      if (left != right)
      {
        return top + u8" " + right + u8" " + bottom + u8" " + left;
      }

      if (bottom != top)
      {
        return top + u8" " + right + u8" " + bottom;
      }

      if (right != top)
      {
        return top + u8" " + right;
      }

      return top;
    }
  };
}