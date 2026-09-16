#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/RectBase.hpp"

namespace krys::boo::css
{
  class Rect final : public RectBase
  {
  public:
    Rect(Ref<Value> top, Ref<Value> right, Ref<Value> bottom, Ref<Value> left)noexcept
        : RectBase(krys::move(top), krys::move(right), krys::move(bottom), krys::move(left))
    {
    }
  };
}