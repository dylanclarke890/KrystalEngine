#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleImage;
}

namespace krys::boo::style
{
  class BuilderState;
}

namespace krys::boo::css
{
  /// @brief The value of a `-webkit-named-image()` function.
  class NamedImageValue final : public Value
  {
  private:
    CSSOMString _name;
    mutable RefPtr<StyleImage> _cachedStyleImage;

    explicit NamedImageValue(CSSOMString &&name) noexcept;

  public:
    ~NamedImageValue() noexcept;

    KRYS_NODISCARD static Ref<NamedImageValue> Create(CSSOMString name) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const NamedImageValue &other) const noexcept;

    KRYS_NODISCARD RefPtr<StyleImage> CreateStyleImage(const style::BuilderState &state) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::NamedImageValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsNamedImageValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
