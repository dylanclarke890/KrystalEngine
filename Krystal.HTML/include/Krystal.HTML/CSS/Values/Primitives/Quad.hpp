#pragma once

#include "Krystal.HTML/CSS/Values/Primitives/RectBase.hpp"

namespace Krys::HTML
{
  class Quad final : public RectBase
  {
  public:
    Quad(Ref<CSSValue> value) : RectBase(Krys::Move(value))
    {
    }
 
    Quad(Ref<CSSValue> top, Ref<CSSValue> right, Ref<CSSValue> bottom, Ref<CSSValue> left)
        : RectBase(Krys::Move(top), Krys::Move(right), Krys::Move(bottom), Krys::Move(left))
    {
    }
  };
}