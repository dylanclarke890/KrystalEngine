#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  class FontFaceSrcLocalValue : public Value
  {
  public:
    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const FontFaceSrcLocalValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(const Func<IterationStatus(css::Value &)> &func) const noexcept;
  };
}

namespace krys::boo::css
{
  class FontFaceSrcResourceValue : public Value
  {
  public:
    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const FontFaceSrcResourceValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(const Func<IterationStatus(css::Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontFaceSrcLocalValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsFontFaceSrcLocalValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontFaceSrcResourceValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsFontFaceSrcResourceValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()