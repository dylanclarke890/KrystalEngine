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
  };
}