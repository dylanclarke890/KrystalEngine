#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/RectBase.hpp"

namespace krys::boo::css
{
  class Quad final : public RectBase
  {
  public:
    Quad(Ref<CSSValue> value) : RectBase(krys::move(value))
    {
    }
 
    Quad(Ref<CSSValue> top, Ref<CSSValue> right, Ref<CSSValue> bottom, Ref<CSSValue> left)
        : RectBase(krys::move(top), krys::move(right), krys::move(bottom), krys::move(left))
    {
    }
  };
}