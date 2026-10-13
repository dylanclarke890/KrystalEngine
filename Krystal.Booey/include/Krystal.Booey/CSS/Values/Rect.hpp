#pragma once

#include "Krystal.Booey/CSS/Values/RectBase.hpp"

namespace krys::boo::css
{
  /// @brief The four sides of a `rect()`, e.g. `clip: rect(1px, 2px, 3px, 4px)`.
  class Rect final : public RectBase
  {
  public:
    Rect(Ref<Value> top, Ref<Value> right, Ref<Value> bottom, Ref<Value> left) noexcept
        : RectBase(krys::move(top), krys::move(right), krys::move(bottom), krys::move(left))
    {
    }

    KRYS_NODISCARD CSSOMString CssText(const SerialisationContext &context) const noexcept
    {
      return GenerateCSSString(Top().CssText(context), Right().CssText(context), Bottom().CssText(context),
                               Left().CssText(context));
    }

  private:
    KRYS_NODISCARD static CSSOMString GenerateCSSString(const CSSOMString &top, const CSSOMString &right,
                                                        const CSSOMString &bottom,
                                                        const CSSOMString &left) noexcept
    {
      return u8"rect(" + top + u8", " + right + u8", " + bottom + u8", " + left + u8')';
    }
  };
}
