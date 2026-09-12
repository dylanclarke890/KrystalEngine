#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/RectBase.hpp"

namespace krys::boo::css
{
  class Rect final : public RectBase
  {
  public:
    Rect(Ref<CSSValue> top, Ref<CSSValue> right, Ref<CSSValue> bottom, Ref<CSSValue> left)noexcept
        : RectBase(krys::move(top), krys::move(right), krys::move(bottom), krys::move(left))
    {
    }
  };
}