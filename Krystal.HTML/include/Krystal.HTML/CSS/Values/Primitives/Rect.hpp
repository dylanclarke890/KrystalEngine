#pragma once

#include "Krystal.HTML/CSS/Values/Primitives/RectBase.hpp"

namespace Krys::HTML
{
  class Rect final : public RectBase
  {
  public:
    Rect(Ref<CSSValue> top, Ref<CSSValue> right, Ref<CSSValue> bottom, Ref<CSSValue> left)noexcept
        : RectBase(Krys::Move(top), Krys::Move(right), Krys::Move(bottom), Krys::Move(left))
    {
    }
  };
}